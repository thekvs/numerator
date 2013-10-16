#include "disk_storage.hpp"

using namespace numerator;

void
usage(const char *program)
{
    std::cerr << "Usage: " << program << " [-h] -d DIR -i FILE" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h       write this help message" << std::endl;
    std::cerr << "  -d DIR   directory with leveldb data files" << std::endl;
    std::cerr << "  -i FILE  file with data to import into leveldb database" << std::endl;
    
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    std::string data_dir;
    std::string data_file;
    int         opt;

    while ((opt = getopt(argc, argv, "d:i:h")) != -1) {
        switch (opt) {
            case 'd':
                data_dir = optarg;
                break;
            case 'i':
                data_file = optarg;
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

    if (data_file.empty()) {
        std::cerr << "Error: mandatory parameter -i is not specified" << std::endl;
        exit(EXIT_FAILURE);
    }

    DiskStorage disk_storage;

    try {
        disk_storage.init(data_dir);
        disk_storage.restore(data_file);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}