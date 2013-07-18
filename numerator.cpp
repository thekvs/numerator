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
    std::cerr << "Usage: " << program << " [-h] [-p PORT] [-d DIR] [-l DIR] [-t NUM]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h                 write this help message" << std::endl;
    std::cerr << "  -p PORT (=9090)    port to bind" << std::endl;
    std::cerr << "  -l DIR (=/tmp)     directory where to write logs" << std::endl;
    std::cerr << "  -d DIR (=/tmp/num) directory where to store data" << std::endl;
    std::cerr << "  -t NUM (=10)       number of worker threads" << std::endl;

    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    std::string data_dir = "/tmp/num";
    std::string logs_dir = "/tmp";
    unsigned    port = kDefaultPort;
    unsigned    threads = kNumThreadsCount;

    int opt;

    while ((opt = getopt(argc, argv, "p:d:l:t:h")) != -1) {
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
            case 'h':
                usage(argv[0]);
                break;
            default:
                std::cerr << "Error: invalid command line argument." << std::endl;
                std::cerr << "Run with -h switch to get help message" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    init_signals();

    LoggerInitializer _logger(logs_dir, argv[0], "numerator");

    LOG(INFO) << "loading data into memory";

    DiskStorage   disk_storage;
    MemoryStorage memory_storage;

    try {
        disk_storage.init(data_dir);
        cnt = disk_storage.load_in_memory(memory_storage);
        cnt++;
    } catch (std::exception &e) {
        LOG(ERROR) << e.what();
        exit(EXIT_FAILURE);
    }

    google::FlushLogFiles(google::INFO);

    boost::shared_ptr<NumeratorHandler>  handler(new NumeratorHandler(memory_storage, disk_storage));
    boost::shared_ptr<TProcessor>        processor(new NumeratorProcessor(handler));
    boost::shared_ptr<TServerTransport>  serverTransport(new TServerSocket(port));
    boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    boost::shared_ptr<TProtocolFactory>  protocolFactory(new TBinaryProtocolFactory());

    boost::shared_ptr<ThreadManager>      threadManager = ThreadManager::newSimpleThreadManager(threads);
    boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());

    threadManager->threadFactory(threadFactory);
    threadManager->start();

    TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);

    pthread_t tid;

    int rc = pthread_create(&tid, NULL, &sigwaiter, &server);
    if (rc != 0) {
        LOG(ERROR) << "pthread_create() failed: " << rc << std::endl;
        exit(EXIT_FAILURE);
    }

    server.serve();

    LOG(INFO) << "done";

    return 0;
}
