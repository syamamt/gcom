#include "stream.hpp"

uint32_t gcom::stream::get_packet(uint32_t idx, unsigned char *payload, uint32_t *head, uint32_t *tail)
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
