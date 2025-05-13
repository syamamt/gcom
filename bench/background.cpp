#include "background.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void background::run(logger& lg)
{
    std::vector<std::thread> threads;
    flag.test_and_set();
    if (config.id == 1) // master
    {
        for (int id = 1; id <= config.threads; id++)
        {
            threads.emplace_back(std::thread(&background::client, this, std::ref(lg), id));
        }

        flag.clear();
        flag.notify_all();
        std::this_thread::sleep_for(std::chrono::seconds(config.duration));
        flag.test_and_set();
    
        for (auto& thread : threads)
        {
            thread.join();
        }
    }
    else // slaves
    {
        threads.emplace_back(std::thread(&background::executer, this, std::ref(lg), 1));

        flag.clear();
        std::this_thread::sleep_for(std::chrono::seconds(config.duration));
        flag.test_and_set();

        threads.back().join();
    }
}

void background::client(logger& lg, int id)
{
    // std::cout << "run client" << id << std::endl;
    int txid = 0;
    int client = id;
    int size = config.log_size;
    flag.wait(true);
    while (!flag.test())
    {
        // op/sec
        auto start = std::chrono::high_resolution_clock::now();
        txid++;
        requests.mtx.lock();
        requests.data.push(transaction(txid, client, size));
        lg.request_transaction(txid, client);
        requests.mtx.unlock();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::this_thread::sleep_for(std::chrono::milliseconds(config.request_interval) - elapsed);
    }
}

void background::executer(logger& lg, int id)
{
    flag.wait(true);
    while (!flag.test())
    {
        requests.mtx.lock();
        if (!requests.data.empty())
        {
            transaction tx = requests.data.front();
            lg.request_transaction(tx.id, tx.client);
            requests.data.pop();
        }
        requests.mtx.unlock();
    }
}