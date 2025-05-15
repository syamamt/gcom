#include "stream_send.hpp"

void gcom::stream_send::push_packets(unsigned char *data, uint32_t len, uint32_t max_payload_size)
{
    uint32_t idx, head, tail;

    idx = buff.push_empty(len);
    head = idx;
    tail = head + len - 1;

    // 分割してバッファに挿入する
    while(idx != head + len)
    {
        uint32_t payload_size = std::min(max_payload_size, head + len - idx);
        buff.set(idx, data + (idx - head), payload_size);
        fprintf(stdout, "insert%" PRIu32 "\n", idx);
        info.emplace(idx, packet_entry(payload_size, head, tail));
        idx += payload_size;
    }

    // 無効な領域のデータのパケット情報を削除する
    for (auto packet = info.begin(); packet != info.end(); packet++)
    {
        if (buff.get_zombie_idx() <= packet->first)
            break;
        
        // ゾンビインデックスより小さいインデックスは無効になる
        packet = info.erase(packet);
    }
}

void gcom::stream_send::pop_packets(uint32_t tail)
{
    uint32_t read_idx = buff.get_read_idx();

    if (read_idx < tail)
    {
        uint32_t pop_len = tail - read_idx;
        buff.pop(pop_len);
    }
}

bool gcom::stream_send::has_ready_packets(uint32_t *idx, uint32_t *len)
{
    decltype(info)::iterator packet = info.find(next_transmit_idx);
    if (packet == info.end())
    {
        return false;
    }

    // 先頭インデックス
    *idx = packet->first;
    
    // 合計データ長の計算
    *len = 0;
    while (packet != info.end())
    {
        *len += packet->second.payload_size;
        packet++;
    }

    return true;
}

void gcom::stream_send::register_transmit_packets(uint32_t idx)
{
    next_transmit_idx = std::max(next_transmit_idx, idx + info.at(idx).payload_size);
}

uint32_t gcom::stream_send::get_remaining_size()
{
    return buff.get_size() - buff.get_write_idx() - buff.get_read_idx();
}