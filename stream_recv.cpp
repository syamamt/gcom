#include "stream_recv.hpp"

void gcom::stream_recv::insert_packet(unsigned char *payload, uint32_t len, uint32_t idx, uint32_t head, uint32_t tail)
{
    // のちに挿入されるインデックスまでバッファの領域を確保する
    if (tail > buff.get_write_idx())
    {
        buff.push_empty(tail - buff.get_write_idx());
    }

    // バッファに該当パケットを格納する
    buff.set(idx, payload, len);

    // infoに該当パケット情報を追加する
    auto result = info.emplace(idx, packet_entry(len, head, tail));
    std::cout << "insert packet() emplace: " << result.second << std::endl;
    if (result.second)
    {
        std::cout << "len: " << result.first->second.payload_size << std::endl;
        std::cout << "head: " << result.first->second.head << std::endl;
        std::cout << "tail: " << result.first->second.tail << std::endl;
    }

    // 新しいデータを挿入したため更新する
    register_next_idx();

    // zombie index未満の位置に格納されていたパケット情報をinfoから削除する
    auto itr = info.begin();
    while (itr != info.end())
    {
        if (buff.get_zombie_idx() <= itr->first)
        {
            break;
        }
        itr = info.erase(itr);
    }
}

uint32_t gcom::stream_recv::try_pop_packets(unsigned char *data)
{
    uint32_t read_idx, tail;

    read_idx = buff.get_read_idx();
    tail = info.at(read_idx).tail;

    // std::cout << "next_idx: " << next_idx << std::endl;

    if (next_idx  > tail)
    {
        uint32_t len = tail - read_idx;
        buff.get(read_idx, data, len);
        buff.pop(len);
        return len;
    }
    else
    {
        return 0;
    }
}

void gcom::stream_recv::register_next_idx()
{
    auto itr = info.find(next_idx);
    while (itr != info.end())
    {
        if (next_idx == itr->first)
        {
            next_idx += itr->second.payload_size;
        }
        else
        {
            break;
        }
    }
}