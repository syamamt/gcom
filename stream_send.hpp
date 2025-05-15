#pragma once

#include "stream.hpp"

namespace gcom
{

    class stream_send : public stream
    {
    public:
        stream_send(uint32_t buffer_size) : stream(buffer_size) {}

        // データをパケットに分割してバッファに挿入する
        void push_packets(unsigned char *data, uint32_t len, uint32_t max_payload_size);

        // 指定したインデックスまでのデータパケットをバッファから削除する
        void pop_packets(uint32_t tail);

        // 新しく送信するデータパケットがバッファに格納されているか確認する
        // 存在する場合、｛先頭インデックス, 合計データ長} も返す
        bool has_ready_packets(uint32_t *idx, uint32_t *len);

        // 送信処理が開始された最新のパケットの先頭インデックスを登録する
        void register_transmit_packets(uint32_t idx);

        // 空いているバッファサイズを取得する
        uint32_t get_remaining_size();
    private:
        // 次に送信するパケットの位置を示すインデックス
        uint32_t next_transmit_idx;
    };

}
