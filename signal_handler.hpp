#ifndef __SIGNAL_HANDLER_HPP_INCLUDED__
#define __SIGNAL_HANDLER_HPP_INCLUDED__

#include <signal.h>
#include <pthread.h>

#include <thrift/server/TThreadPoolServer.h>

#include "logger_initializer.hpp"

namespace numerator {

void init_signals();
void* sigwaiter(void *arg);

} // namespace

#endif
