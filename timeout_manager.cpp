#include "timeout_manager.hpp"

gcom::timeout_manager::timeout_manager()
{
    tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
}

gcom::timeout_manager::~timeout_manager()
{
    close(tfd);
}
