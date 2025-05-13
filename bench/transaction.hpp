#pragma once

#include <queue>
#include <mutex>

class transaction
{
public:
    transaction() {}
    transaction(int id, int client, int size) : id(id), client(client), size(size) {}

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(id, client, size);
    }

    void print()
    {
        printf("tx id:%d client:%d size:%d\n", id, client, size);
    }

    int id;
    int client;
    int size;
};

class txqueue
{
public:
    std::queue<transaction> data;
    std::mutex mtx;
};