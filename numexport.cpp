#include "disk_storage.hpp"

using namespace numerator;

void
usage(const char *program)
{
    std::cerr << "Usage: " << program << " [-h] -d DIR [-o FILE]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  -h       write this help message" << std::endl;
    std::cerr << "  -d DIR   directory with leveldb data files" << std::endl;
    std::cerr << "  -o FILE  file where to output data" << std::endl;
    std::cerr << "  -b       produce binary dump" << std::endl;
    
    exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
    std::string data_dir;
    std::string output_file;
    int         opt;
    bool        binary_dump = false;

    while ((opt = getopt(argc, argv, "d:o:hb")) != -1) {
        switch (opt) {
            case 'd':
                data_dir = optarg;
                break;
            case 'o':
                output_file = optarg;
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

    DiskStorage disk_storage;

    try {
        disk_storage.init(data_dir);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!output_file.empty()) {
        std::ios_base::openmode mode = std::ios::trunc;
        if (binary_dump) {
            mode |= std::ios::binary;
        }
        std::ofstream stream(output_file.c_str(), mode);
        if (stream.fail()) {
            std::cerr << "Error: couldn't open file \"" << output_file << "\" for writing" << std::endl;
            exit(EXIT_FAILURE);
        }
        disk_storage.dump(stream, binary_dump);
    } else {
        disk_storage.dump(std::cout);
    }

    return EXIT_SUCCESS;
}
