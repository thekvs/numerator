#include "signal_handler.hpp"

namespace numerator {

static sigset_t set;

using namespace apache::thrift::server;

void
init_signals()
{
    sigemptyset(&set);

    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);

    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void*
sigwaiter(void *arg)
{
    if (arg == NULL) {
        LOG(FATAL) << "Invalid argument";
    }

    int status, signum;

    TThreadPoolServer *server = static_cast<TThreadPoolServer*>(arg);

    LOG(INFO) << "started signal handling thread";

    while (true) {
        status = sigwait(&set, &signum);
        if (status == 0) {
            if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP) {
                LOG(WARNING) << "received " << signum << " signal, exiting";
                server->stop();
                break;
            }
        }
    }

    LOG(INFO) << "finished signal handling thread";

    return NULL;
}

} // namespace
