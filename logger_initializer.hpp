#ifndef __LOGGER_INITIALIZER_HPP_INCLUDED__
#define __LOGGER_INITIALIZER_HPP_INCLUDED__

#include <string>

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
    }

    ~LoggerInitializer()
    {
        google::FlushLogFiles(google::INFO);
        google::ShutdownGoogleLogging();
    }

private:

    LoggerInitializer() {};
};

} // namespace

#endif
