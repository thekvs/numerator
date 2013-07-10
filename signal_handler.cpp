#include "signal_handler.hpp"

namespace numerator {

static sigset_t set;

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
    int status, signum;

    using namespace apache::thrift::server;

    TThreadPoolServer *server = static_cast<TThreadPoolServer*>(arg);

    LOG(INFO) << "started sigwaiter thread";

    while (true) {
        status = sigwait(&set, &signum);
        if (status == 0) {
            if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP) {
                LOG(WARNING) << "received " << signum << " signal, exiting";
                server->stop();
            }
        }
    }

    return NULL;
}

} // namespace
