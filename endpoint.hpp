#pragma once

#include <string>

namespace gcom
{

    class endpoint
    {
    public:
        endpoint(std::string ipaddr, uint16_t port, bool recovery)
            : ipaddr(ipaddr), port(port), recovery(recovery) {}
        endpoint() : ipaddr("0.0.0.0"), port(0), recovery(false) {}

        endpoint& operator=(const endpoint& ep) = default;

        bool operator<(const endpoint& other) const
        {
            if (ipaddr != other.ipaddr)
                return ipaddr < other.ipaddr;
            else
                return port < other.port;
        }

        const std::string& get_ipaddr() const { return ipaddr; }
        uint16_t get_port() const { return port; }
        bool is_recovery() const { return recovery; }

    private:
        std::string ipaddr; // IPアドレス
        uint16_t port; // ポート番号
        bool recovery; // リカバリーノード
    };

} // namespace gcom
