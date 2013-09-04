#include "disk_storage.hpp"

using namespace numerator;

void
usage(const char *program)
{
    std::cerr << "Usage: " << program << " [-h] -d DIR" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h      write this help message" << std::endl;
    std::cerr << "  -d DIR  directory with leveldb data files" << std::endl << std::endl;
    
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    std::string data_dir;
    int         opt;

    while ((opt = getopt(argc, argv, "d:h")) != -1) {
        switch (opt) {
            case 'd':
                data_dir = optarg;
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

    DiskStorage disk_storage;

    try {
        disk_storage.init(data_dir);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    disk_storage.dump(std::cout);

    return EXIT_SUCCESS;
}
