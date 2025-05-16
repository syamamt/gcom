#include "gcom.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

constexpr char lo_ipaddr[] = "127.0.0.1";
constexpr int base_port = 10000;
constexpr int recievers = 2;
constexpr int generate_string_size = 2048;
constexpr int loops = 100;


std::string generate_random_string(size_t length) {
    const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    // シードを固定
    std::mt19937 engine(100);
    std::uniform_int_distribution<> dist(0, charset.size() - 1);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(engine)];
    }
    return result;
}

void sender(int self_nodeid) {
    gcom::socket sock(base_port + self_nodeid);

    // クラスタノード情報の設定
    for (int nodeid = 0; nodeid <= recievers; nodeid++) {
        if (nodeid == self_nodeid) {
            continue;
        }
        else {
            gcom::endpoint ep(std::string(lo_ipaddr), base_port + nodeid, false);
            sock.add_group(ep);
        }
    }

    sock.open();

    // send_allテスト
    for (int i = 0; i < 100; i++) {
        std::string msg = generate_random_string(generate_string_size);
        sock.async_send_all(msg.c_str(), msg.size());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    sock.close();
}

void reciever(int self_nodeid) {
    gcom::socket sock(base_port + self_nodeid);

    // クラスタノード情報の設定
    for (int nodeid = 0; nodeid <= recievers; nodeid++) {
        if (nodeid == self_nodeid) {
            continue;
        }
        else if(nodeid == 0) {
            gcom::endpoint ep(std::string(lo_ipaddr), base_port + nodeid, false);
            sock.add_group(ep);
        }
        else {
            gcom::endpoint ep(std::string(lo_ipaddr), base_port + nodeid, true);
            sock.add_group(ep);
        }
    }

    sock.open();

    // recv_fromテスト
    gcom::endpoint ep;
    for (int i = 0; i < 100; i++) {
        char msg[5120];
        int len = sock.async_recv_from(msg, &ep);
        msg[len] = '\0';

        if (std::string(msg) == generate_random_string(generate_string_size)) {
            fprintf(stdout, "node%d, success recv_from(): %d\n", self_nodeid, i);
        }
        else {
            fprintf(stdout, "node%d, fail recv_from(): %d\n", self_nodeid, i);
        }
    }

    sock.close();
}

int main() {
    std::vector<std::thread> nodes;

    // recieverの起動
    for (int nodeid = 1; nodeid <= recievers; nodeid++) {
        nodes.emplace_back(reciever, nodeid);
    }

    // senderの起動
    nodes.emplace_back(sender, 0);

    for (auto& n : nodes) {
        if (n.joinable()) {
            n.join();
        }
    }

    return 0;
}