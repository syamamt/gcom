#pragma once

#include "configuration.hpp"
#include "transaction.hpp"
#include "logger.hpp"
#include <queue>
#include <atomic>

class background
{
public:
    background(configuration& _config, txqueue& _requests)
        : config(_config), requests(_requests), flag(ATOMIC_FLAG_INIT) {}
    void run(logger& lg);
private:
    configuration& config;
    txqueue& requests;
    std::atomic_flag flag; // lock: not-started terminated, unlock: started

    void client(logger& lg, int id);
    void executer(logger& lg, int id);
};
