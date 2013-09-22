#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#include "gen-cpp/Numerator.h"
#include "disk_storage.hpp"
#include "memory_storage.hpp"
#include "logger_initializer.hpp"
#include "signal_handler.hpp"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace numerator;

static NumID cnt;

static const unsigned kNumThreadsCount = 10;
static const unsigned kDefaultPort = 9090;

class NumeratorHandler: virtual public NumeratorIf {
public:

    NumeratorHandler(MemoryStorage &_memory_storage, DiskStorage &_disk_storage):
        memory_storage(_memory_storage),
        disk_storage(_disk_storage)
    {
    }

    void query(Query& response, const Query& request)
    {
        try {
            query_impl(response, request);
        } catch (std::exception &e) {
            LOG(ERROR) << e.what();
            exit(EXIT_FAILURE);
        }
    }

    void ping() {}

private:

    MemoryStorage &memory_storage;
    DiskStorage   &disk_storage;

    void query_impl(Query& response, const Query& request)
    {
        // Looks like every service's method acquires global lock (if we use threads),
        // so there is no need for another synchronization/lock mechanism.
        if (request.op == Operation::ID2STR) {
            disk_storage.lookup(request.ids, response.strings, response.failures);
        } else {
            size_t i;
            size_t count = request.strings.size();

            KVPairs kv_pairs;
            bool    inserted;
            NumID   id;

            for (i = 0; i < count; i++) {
                const std::string &str = request.strings[i];
                boost::tie(inserted, id) = memory_storage.find(str, cnt);
                if (inserted) {
                    kv_pairs.push_back(KVPair(id, str));
                }
                response.ids.push_back(id);
            }

            disk_storage.write(kv_pairs);
        }

        response.op = request.op;
    }
};

void
usage(const char *program)
{
    std::cerr << "Usage: " << program << " [-h] [-p PORT] [-d DIR] [-l DIR] [-t NUM] [-c SIZE]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h                 write this help message" << std::endl;
    std::cerr << "  -l DIR             directory where to write logs" << std::endl;
    std::cerr << "  -d DIR             directory where to store data" << std::endl;
    std::cerr << "  -t NUM (=10)       number of worker threads" << std::endl;
    std::cerr << "  -p PORT (=9090)    port to bind" << std::endl;
    std::cerr << "  -c SIZE            id to string lookups cache size (in MB)," << std::endl;
    std::cerr << "                     if not specified use leveldb's default value" << std::endl;

    exit(EXIT_SUCCESS);
}

bool
is_dir(std::string path)
{
    struct stat st;

    memset(&st, 0, sizeof(st));

    int rc = stat(path.c_str(), &st);
    if (rc != 0) {
        if (errno == ENOENT) {
            return false;
        } else {
            std::cerr << "Error: stat() failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return S_ISDIR(st.st_mode) ? true : false;
}

void
thrift_messages_logger(const char *message)
{
    LOG(WARNING) << message;
}

void*
logs_flusher(void*)
{
    while (true) {
        google::FlushLogFiles(google::INFO);
        sleep(1);
    }

    return NULL;
}

int
main(int argc, char **argv)
{
    std::string data_dir;
    std::string logs_dir;
    unsigned    port = kDefaultPort;
    unsigned    threads = kNumThreadsCount;
    size_t      cache_size = 0;

    int opt, rc;

    while ((opt = getopt(argc, argv, "p:d:l:t:c:h")) != -1) {
        switch (opt) {
            case 'p':
                try {
                    port = boost::lexical_cast<unsigned>(optarg);
                } catch (std::exception &e) {
                    std::cerr << "Error: invalid argument for -p switch" << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                data_dir = optarg;
                break;
            case 'l':
                logs_dir = optarg;
                break;
            case 't':
                try {
                    threads = boost::lexical_cast<unsigned>(optarg);
                } catch (std::exception &e) {
                    std::cerr << "Error: invalid argument for -t switch" << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
                try {
                    cache_size = boost::lexical_cast<size_t>(optarg);
                } catch (std::exception &e) {
                    std::cerr << "Error: invalid argument for -c switch" << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                usage(argv[0]);
                break;
            default:
                std::cerr << "Error: invalid command line argument." << std::endl;
                std::cerr << "Run with -h switch to get help message" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    if (data_dir.empty()) {
        std::cerr << "Error: mandatory parameter -d is not specified" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (logs_dir.empty()) {
        std::cerr << "Error: mandatory parameter -l is not specified" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!is_dir(logs_dir)) {
        std::cerr << "Error: " << logs_dir << " is not a directory or does not exist" << std::endl;
        exit(EXIT_FAILURE);
    }

    init_signals();

    LoggerInitializer _logger(logs_dir, argv[0], "numerator");

    LOG(INFO) << "loading data into memory";

    DiskStorage   disk_storage;
    MemoryStorage memory_storage;

    try {
        disk_storage.init(data_dir, cache_size);
        cnt = disk_storage.load_in_memory(memory_storage);
        cnt++;
    } catch (std::exception &e) {
        LOG(ERROR) << e.what();
        exit(EXIT_FAILURE);
    }

    pthread_t logs_flusher_tid;

    rc = pthread_create(&logs_flusher_tid, NULL, &logs_flusher, NULL);
    if (rc != 0) {
        LOG(ERROR) << "pthread_create() failed: " << rc << std::endl;
        exit(EXIT_FAILURE);
    }

    // Route messages from Thrift internals to the log files
    GlobalOutput.setOutputFunction(thrift_messages_logger);

    boost::shared_ptr<NumeratorHandler>  handler(new NumeratorHandler(memory_storage, disk_storage));
    boost::shared_ptr<TProcessor>        processor(new NumeratorProcessor(handler));
    boost::shared_ptr<TServerTransport>  server_transport(new TServerSocket(port));
    boost::shared_ptr<TTransportFactory> transport_factory(new TBufferedTransportFactory());
    boost::shared_ptr<TProtocolFactory>  protocol_factory(new TBinaryProtocolFactory());

    boost::shared_ptr<ThreadManager>      thread_manager = ThreadManager::newSimpleThreadManager(threads);
    boost::shared_ptr<PosixThreadFactory> thread_factory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());

    thread_manager->threadFactory(thread_factory);
    thread_manager->start();

    TThreadPoolServer server(processor, server_transport, transport_factory, protocol_factory, thread_manager);

    pthread_t sigwaiter_tid;

    rc = pthread_create(&sigwaiter_tid, NULL, &sigwaiter, &server);
    if (rc != 0) {
        LOG(ERROR) << "pthread_create() failed: " << rc << std::endl;
        exit(EXIT_FAILURE);
    }

    server.serve();

    LOG(INFO) << "done";

    return 0;
}
