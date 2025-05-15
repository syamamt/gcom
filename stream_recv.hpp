#pragma once

#include "stream.hpp"

namespace gcom
{

    class stream_recv : public stream
    {
    public:
        stream_recv(uint32_t buffer_size) : stream(buffer_size), next_recv_packet_idx(0) {}

        // バッファにデータパケットを挿入
        void insert_packet(unsigned char *payload, uint32_t len, uint32_t idx, uint32_t head, uint32_t tail);

        // バッファからconfirmed packetsを削除
        uint32_t try_pop_packets(unsigned char *data);

    private:
        // 確認できたインデックスを登録する
        void register_next_recv_packet_idx();

        // 次に受信したいパケットのインデックス
        // (next_recv_packet_idx - 1)までのパケットの受信に成功している
        uint32_t next_recv_packet_idx;
    };

}
