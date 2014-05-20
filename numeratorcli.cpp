#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "exc.hpp"
#include "gen-cpp/Numerator.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace numerator;

static struct option longopts[] = {
    { "numerator",  required_argument,  NULL,   'n' },
    { "type",       required_argument,  NULL,   't' },
    { "data",       required_argument,  NULL,   'd' },
    { "batch-size", required_argument,  NULL,   'b' },
    { "help",       no_argument,        NULL,   'h' },
    { NULL,         0,                  NULL,   0   }
};

static const size_t      kDefaultBatchSize = 1000;
static const std::string kDefaultNumeratorAddress = "127.0.0.1:9090";

typedef std::vector<NumID>       IDVector;
typedef std::vector<std::string> StringVector;

class CmdLineArgs {
public:

    std::string numerator;
    std::string type;
    std::string data;
    size_t    batch_size;

    CmdLineArgs(std::string _numerator, std::string _type, std::string _data, size_t _batch_size):
        numerator(_numerator),
        type(_type),
        data(_data),
        batch_size(_batch_size)
    {}
};

void
usage(const char *program)
{
    std::cerr << "Usage: " << program << " args" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h,--help            write this help message and exit" << std::endl;
    std::cerr << "  -n,--numerator arg   numerator's address (location:port, default 127.0.0.1:9090)" << std::endl;
    std::cerr << "  -t,--type arg        type of the query ('s2i' or 'i2s')" << std::endl;
    std::cerr << "  -d,--data arg        file with data (one string per line)" << std::endl;
    std::cerr << "  -b,--batch-size arg  number of items in one request" << std::endl;
    std::cerr << std::endl;

    exit(EXIT_SUCCESS);
}

CmdLineArgs
parse_cmd_line_args(int argc, char **argv)
{
    int opt, optidx;

    std::string numerator = kDefaultNumeratorAddress;
    size_t      batch_size = kDefaultBatchSize;
    std::string type;
    std::string data;

    while ((opt = getopt_long(argc, argv, "d:n:t:b:h", longopts, &optidx)) != -1) {
        switch (opt) {
            case 'd':
                data = optarg;
                break;
            case 'n':
                numerator = optarg;
                break;
            case 't':
                type = optarg;
                break;
            case 'b':
                try {
                    batch_size = boost::lexical_cast<size_t>(optarg);
                } catch (std::exception& exc) {
                    THROW_EXC("Invalid batch size '%s': %s", optarg, exc.what());
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

    if (numerator.empty()) {
        std::cerr << "Error: mandatory parameter -n is not specified." << std::endl;
        std::cerr << "Run with -h switch to get help message." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (data.empty()) {
        std::cerr << "Error: mandatory parameter -d is not specified." << std::endl;
        std::cerr << "Run with -h switch to get help message." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (type.empty()) {
        std::cerr << "Error: mandatory parameter -t is not specified." << std::endl;
        std::cerr << "Run with -h switch to get help message." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (type != "s2i" && type != "i2s") {
        std::cerr << "Error: invalid argument for 'type' arg. Has to be either 's2i' or 'i2s'." << std::endl;
        exit(EXIT_FAILURE);
    }

    THROW_EXC_IF_FAILED(batch_size > 0, "batch size has to be >0.");

    CmdLineArgs args(numerator, type, data, batch_size);

    return args;
}

std::pair<std::string, int>
split_host_port(const CmdLineArgs& args)
{
    std::vector<std::string> tokens;
    boost::split(tokens, args.numerator, boost::is_any_of(":"), boost::token_compress_on);

    THROW_EXC_IF_FAILED(tokens.size() == 2, "Invalid format of numerator service address: %s", args.numerator.c_str());

    std::string host = tokens[0];
    int         port;

    try {
        port = boost::lexical_cast<int>(tokens[1]);
    } catch (std::exception &exc) {
        THROW_EXC("Invalid format of numerator service address '%s': %s", args.numerator.c_str(), exc.what());
    }

    return std::make_pair(host, port);
}

void
do_query(NumeratorClient& client, Operation::type op, const IDVector& ids, const StringVector& strings)
{
    Query request, result;

    request.op = op;
    request.strings = strings;
    request.ids = ids;

    client.query(result, request);

    if (op == Operation::STR2ID) {
        BOOST_FOREACH(const IDVector::value_type &v, result.ids) {
            std::cout << v << std::endl;
        }
    } else {
        FailureIdx           idx = 0;
        std::set<FailureIdx> failures(result.failures.begin(), result.failures.end());

        BOOST_FOREACH(const StringVector::value_type &v, result.strings) {
            if (failures.find(idx) != failures.end()) {
                std::cout << "!!! NOT FOUND !!!" << std::endl;
            } else {
                std::cout << v << std::endl;
            }
            idx++;
        }
    }
}

void
run(const CmdLineArgs& args, NumeratorClient& client)
{
    std::ifstream stream(args.data.c_str(), std::ios::in);
    THROW_EXC_IF_FAILED(!stream.fail(), "couldn't open file %s", args.data.c_str());

    Operation::type op;

    if (args.type == "s2i") {
        op = Operation::STR2ID;
    } else {
        op = Operation::ID2STR;
    }

    StringVector strings;
    IDVector     ids;

    strings.reserve(args.batch_size);
    ids.reserve(args.batch_size);

    std::string line;
    size_t      cnt;

    while (std::getline(stream, line)) {
        if (op == Operation::STR2ID) {
            strings.push_back(line);
        } else if (op == Operation::ID2STR) {
            ids.push_back(boost::lexical_cast<NumID>(line));
        }
        cnt++;
        if (cnt >= args.batch_size) {
            do_query(client, op, ids, strings);
            cnt = 0;
            ids.clear();
            strings.clear();        
        }
    }

    if (cnt > 0) {
        do_query(client, op, ids, strings);
    }
}

int
main(int argc, char **argv)
{
    try {
        CmdLineArgs args = parse_cmd_line_args(argc, argv);

        std::string host;
        int         port;

        boost::tie(host, port) = split_host_port(args);

        boost::shared_ptr<TSocket>    socket(new TSocket(host, port));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        boost::shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));

        transport->open();

        NumeratorClient client(protocol);

        client.ping();
        run(args, client);
        transport->close();
    } catch (NumeratorException& exc) {
        std::cerr << "Error: " << exc.message << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Error occured." << std::endl;
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
