#ifndef __LOGGER_INITIALIZER_HPP_INCLUDED__
#define __LOGGER_INITIALIZER_HPP_INCLUDED__

#include <string>
#include <pthread.h>

#include <glog/logging.h>

namespace numerator {

class LoggerInitializer {
public:

    LoggerInitializer(std::string logs_dir, std::string name, std::string base_filename)
    {
        std::string s, dir;

        dir = logs_dir;

        if (dir[dir.size()] != '/') {
            dir += "/";
        }

        google::InitGoogleLogging(name.c_str());

        s = dir + base_filename + ".log.ERROR.";
        google::SetLogDestination(google::ERROR, s.c_str());

        s = dir + base_filename + ".log.INFO.";
        google::SetLogDestination(google::INFO, s.c_str());

        s = dir + base_filename + ".log.WARNING.";
        google::SetLogDestination(google::WARNING, s.c_str());

        pthread_t tid;
        int       rc;

        rc = pthread_create(&tid, NULL, &logs_flusher, NULL);
        if (rc != 0) {
            LOG(ERROR) << "pthread_create() failed: " << rc << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ~LoggerInitializer()
    {
        google::FlushLogFiles(google::INFO);
        google::ShutdownGoogleLogging();
    }

private:

    LoggerInitializer() {};

    static void* logs_flusher(void*)
    {
        while (true) {
            google::FlushLogFiles(google::INFO);
            sleep(1);
        }

        return NULL;
    }

};

} // namespace

#endif
