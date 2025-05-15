#include "stream_recv.hpp"

void gcom::stream_recv::insert_packet(unsigned char *payload, uint32_t len, uint32_t idx, uint32_t head, uint32_t tail)
{
    // のちに挿入されるインデックスまでバッファの領域を確保する
    if (tail + 1 > buff.get_write_idx())
    {
        buff.push_empty(tail + 1 - buff.get_write_idx());
    }

    // バッファに該当パケットを格納する
    buff.set(idx, payload, len);

    // infoに該当パケット情報を追加する
    info.emplace(idx, packet_entry(len, head, tail));

    // 新しいデータを挿入したため更新する
    register_next_recv_packet_idx();

    // zombie index未満の位置に格納されていたパケット情報をinfoから削除する
    auto packet = info.begin();
    while (packet != info.end())
    {
        if (buff.get_zombie_idx() <= packet->first)
        {
            break;
        }

        // ゾンビインデックスより小さいインデックスは無効になる
        packet = info.erase(packet);
    }
}

uint32_t gcom::stream_recv::try_pop_packets(unsigned char *data)
{
    uint32_t head = buff.get_read_idx();

    auto packet = info.find(head);
    if (packet == info.end()) {
        return 0;
    }
    uint32_t tail = packet->second.tail;

    if (next_recv_packet_idx  > tail)
    {
        uint32_t len = tail + 1 - head;
        buff.get(head, data, len);
        buff.pop(len);
        return len;
    }
    else
    {
        return 0;
    }
}

void gcom::stream_recv::register_next_recv_packet_idx()
{
    auto packet = info.find(next_recv_packet_idx);
    while (packet != info.end())
    {
        if (next_recv_packet_idx == packet->first)
        {
            next_recv_packet_idx += packet->second.payload_size;
        }
        else
        {
            break;
        }
    }
}