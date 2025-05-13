#pragma once

#include "configuration.hpp"
#include "transaction.hpp"
#include "logger.hpp"
#include <queue>
#include <thread>

class tcp_connection_manager
{
public:
    tcp_connection_manager(configuration& config, txqueue& requests, logger& lg);
    ~tcp_connection_manager();
private:
    configuration& config;
    txqueue& requests;
    logger& lg;
    std::thread worker;
    int connect_fd[10];
    int listen_fd;

    std::atomic_flag flag; // lock: terminated, unlock: started

    void sender();
    void receiver();

    enum message_offset
    {
        offset_size = 0,
        offset_data = offset_size + sizeof(int),
    };

    int create_message(int len, char *msg, int msgsize, std::string data);
    int analyze_message(char *c_data, char *msg);
};