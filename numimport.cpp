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
    std::cerr << "  -b       restore from binary dump" << std::endl;
    
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    std::string data_dir;
    std::string data_file;
    int         opt;
    bool        binary_dump = false;

    while ((opt = getopt(argc, argv, "d:i:hb")) != -1) {
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
            case 'b':
                binary_dump = true;
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
        std::ios_base::openmode mode = std::ios::in;
        if (binary_dump) {
            mode |= std::ios::binary;
        }
        std::ifstream stream(data_file.c_str(), mode);
        THROW_EXC_IF_FAILED(!stream.fail(), "Couldn't open data stream \"%s\"", data_file.c_str());
        disk_storage.restore(stream, binary_dump);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
