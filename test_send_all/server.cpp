#include "gcom.hpp"
#include <iostream>
#include <string>
#include <vector>

// <id> <self port> <n1 ipaddr> <n1 port> <n1 is recovery> ...

int main(int argc, char *argv[]) {
    if (argc < 6 || (argc - 3) % 3 != 0) {
        std::cerr << "Usage: " << argv[0] << " <id> <port> <n1 ipaddr> <n1 port> <n1 is recovery> ..." << std::endl;
        return 1;
    }

    // ノードID：0なら送信ノード、それ以外なら受信ノード
    uint16_t id = static_cast<uint16_t>(std::stoi(argv[1]));

    // 自身のポート番号
    uint16_t self_port = static_cast<uint16_t>(std::stoi(argv[2]));
    gcom::socket sock(self_port);

    // 別ノードの宛先情報を登録する
    std::vector<gcom::endpoint> nodes;
    for (int i = 3; i + 2 < argc; i += 3) {
        std::string ipaddr = argv[i];
        uint16_t port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
        bool is_recovery = !(static_cast<int>(std::stoi(argv[i + 2])) == 0);
        gcom::endpoint e(ipaddr, port, is_recovery);
        nodes.push_back(e);

        sock.add_group(e);
    }

    // 設定を表示する
    std::cout << "ID: " << id << ", Self Port: " << self_port << std::endl;
    for (size_t i = 0; i < nodes.size(); i++) {
        const auto& n = nodes[i];
        std::cout << "Cluster Node " << i + 1 << ": IP = " << n.get_ipaddr()
                  << ", Port = " << n.get_port()
                  << ", Is Recovery Node = " << (n.is_recovery() ? "true" : "false") << std::endl;
    }

    // テスト本体
    if (id == 0) { // 送信ノード
        char msg[] = "abcedfg";
        sock.send_all(msg, strlen(msg));
    } else { // 受信ノード
        char msg[10];
        gcom::endpoint ep;
        sock.recv_from(msg, &ep);
        std::cout << msg << std::endl;
    }

    std::cout << "fin" << std::endl;

    return 0;
}