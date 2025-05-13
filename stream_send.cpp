#include "stream_send.hpp"

void gcom::stream_send::push_packets(unsigned char *data, uint32_t len, uint32_t max_payload_size)
{
    uint32_t idx, head, tail;

    idx = buff.push_empty(len);
    head = idx;
    tail = head + len - 1;

    // 分割してバッファに挿入する
    while(idx != tail + 1)
    {
        uint32_t payload_size = std::min(max_payload_size, tail + 1 - idx);
        buff.set(idx, data + (idx - head), payload_size);
        info.emplace(idx, packet_entry(payload_size, head, tail));
        idx += payload_size;
    }

    // 無効な領域のデータのパケット情報を削除する
    for (auto itr = info.begin(); itr != info.end(); itr++)
    {
        // todo
        if (buff.get_zombie_idx() <= itr->first)
        {
            break;
        }
        itr = info.erase(itr);
    }
}

void gcom::stream_send::pop_packets(uint32_t tail)
{
    uint32_t read_idx;

    read_idx = buff.get_read_idx();

    if (read_idx < tail)
    {
        uint32_t len = tail - read_idx;
        buff.pop(len);
    }

    next_transmit_idx = tail;
}

bool gcom::stream_send::has_ready_packets(uint32_t *idx, uint32_t *len)
{
    decltype(info)::iterator itr = info.find(next_transmit_idx);
    if (itr == info.end())
    {
        return false;
    }

    // 先頭インデックス
    *idx = next_transmit_idx;
    
    // 合計データ長の計算
    *len = 0;
    while (itr != info.end())
    {
        *len += itr->second.payload_size;
        itr++;
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