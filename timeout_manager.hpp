#pragma once

#include <sys/timerfd.h>
#include <unistd.h>
#include <map>

namespace gcom
{
    class timeout_manager
    {
    public:
        timeout_manager();
        ~timeout_manager();

        int add(uint32_t streamid);
        uint32_t get_streamid(int tfd);
    private:
        std::map<int, uint32_t> tfds; // tfd: streamid
    };

    int timeout_manager::add(uint32_t streamid)
    {
        int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        tfds.insert(tfd, streamid);
        return tfd;
    }
    
    uint32_t timeout_manager::get_streamid(int tfd)
    {
        return tfds.at(tfd);
    }
}
