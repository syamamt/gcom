#pragma once

#include "ring_buffer.hpp"
#include "endpoint.hpp"
#include <iostream>
#include <map>
#include <cinttypes>

namespace gcom
{

    class stream
    {
    public:
        stream(uint32_t buffer_size) : buff(buffer_size) {}

        // バッファから指定されたインデックスのパケットを取得する
        uint32_t get_packet(uint32_t idx, unsigned char *payload, uint32_t *head, uint32_t *tail);

    protected:
        struct packet_entry
        {
            packet_entry(uint32_t payload_size, uint32_t head, uint32_t tail)
                : payload_size(payload_size), head(head), tail(tail) {}

            uint32_t payload_size;
            uint32_t head;
            uint32_t tail;
        };

        // バッファ本体
        ring_buffer<uint32_t> buff;

        // バッファ内の有効なデータをパケット単位で管理する
        // キー：書き込み位置を示すインデックス
        std::map<uint32_t, packet_entry> info;
    };

    uint32_t stream::get_packet(uint32_t idx, unsigned char *payload, uint32_t *head, uint32_t *tail)
    {
        auto itr = info.find(idx);
        if (itr == info.end())
        {
            throw std::exception();
        }

        *head = itr->second.head;
        *tail = itr->second.tail;
        buff.get(idx, payload, itr->second.payload_size);
        return itr->second.payload_size;
    }
}
