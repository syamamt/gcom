#pragma once

#include "stream_send.hpp"
#include "stream_recv.hpp"
#include "endpoint.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <map>
#include <set>
#include <exception>
#include <iostream>
#include <atomic>
#include <shared_mutex>
#include <condition_variable>
#include <random>
#include <optional>
#include <cinttypes>

#define NUM_UNICAST_STREAMS 1
#define NUM_MULTICAST_STREAMS 1
#define STREAM_BUFFER_SIZE 8388608 // 8[MB]：TCP/IPバッファサイズと同値

namespace gcom
{
    struct header
    {
        uint32_t streamid; // stream id
        uint32_t seq;
        uint32_t head;
        uint32_t tail;
        uint8_t flag;
    };

    enum class header_flag : uint8_t
    {
        FLG_CTL_ACK,        // ACKパケット
        FLG_CTL_NCK,        // NACKパケット
        FLG_DAT_RTM,        // 再送データパケット
        FLG_DAT_UNI,        // ユニキャストデータパケット
        FLG_DAT_MLT_SRC,    // ソースからのマルチキャストデータパケット
        FLG_DAT_MLT_RCV,    // 同一グループからのマルチキャストリカバリ
    };

    static const int max_epoll_events = 16;
    static const int epoll_wait_timeout = 0; // [ms]
    static const int interval_for_querying_recv_buffer = 0; // [ms]
    static const int num_unicast_streams = NUM_UNICAST_STREAMS; // 各送信者のユニキャスト多重ストリーム数
    static const int num_multicast_streams = NUM_MULTICAST_STREAMS; // 各送信者のマルチキャスト多重ストリーム数
    static const int stream_buffer_size = STREAM_BUFFER_SIZE; // [bytes]
    static const int packet_size = 1472; // [bytes] 最大パケットサイズ（最大UDPペイロードサイズ）
    static const int payload_size = packet_size - sizeof(header); // [bytes] 最大ペイロードサイズ

    class socket
    {
    public:
        socket(uint16_t port);
        ~socket();

        // バックグラウンドスレッドを起動する
        // 呼出後、送受信関数を受け付けられる
        void open();

        // バックグラウンドスレッドを終了する
        void close();

        // // destで指定した宛先にデータを送信する
        // void send_to(const void *buf, size_t len, endpoint& dest);

        // クラスタ内の全ノードにデータを送信する
        void async_send_all(const void *buf, size_t len);

        // クラスタ内の指定したノードにデータを受信する
        uint32_t async_recv_from(void *buf, endpoint *from);

        // クラスタにノードを登録する（各ノードがそれぞれ管理）
        // 登録されていないノードからのデータ受信、ノードへのデータ送信は無視される
        void add_group(endpoint& ep);

    private:
        // バックグラウンドスレッド
        void background();

        // 受信パケットの処理
        void handle_arriving_packet();

        // 送信バッファ内のデータを送信する
        void transmit_ready_packets();

        // // 再送
        // void retransmit_packets(int streamid);

        // // ACKパケットを送信する
        // void transmit_ack_packet();

        // // NACKパケットを送信する
        // void transmit_nack_packet();

        // // グループリカバリパケットを送信する
        // void transmit_group_recovery_packet(int streamid, int seq);

        // UDPソケットを用いてパケットを出力する
        void output_packet(const struct header* hdr, const void *payload, size_t payload_len, endpoint to_ep);

        // UDPソケットを用いてパケットを入力する
        std::pair<size_t, endpoint> input_packet(struct header* hdr, void *payload);

        // epollfdに監視するfdを登録
        void register_epoll_event(int fd);

        // ストリームIDとしてランダムな値を取得する
        uint32_t get_streamid();

        int sockfd; // UDPソケットファイルディスクリプタ
        int epollfd; // epollイベントファイルディスクリプタ
        std::atomic_flag system_state; // true->起動中, false->停止中
        std::thread bgthread; // バックグラウンドスレッド

        std::set<endpoint> cluster; // クラスタ内の全ノード
        // std::map<decltype(cluster)::iterator, std::set<int>> unicast_streamid; // ユニキャストストリームIDの管理

        // struct unicast_info
        // {
        //     decltype(cluster)::iterator ep_itr;
        //     stream_send body;

        //     // timeout manager
        // };
        // using UnicastStreamMap = std::map<int, struct unicast_info>; // key: ストリームID
        // UnicastStreamMap unicast_stream_map; // ユニキャストストリーム

        struct multicast_info
        {
            stream_send buff;
            // timeout manger

            multicast_info(int size) : buff(size) {}
        };
        using MulticastStreamMap = std::map<uint32_t, multicast_info>; // key: ストリームID
        MulticastStreamMap multicast_stream_map; // マルチキャストストリーム
        std::shared_mutex multicast_stream_mutex;

        struct recv_info
        {
            decltype(cluster)::iterator ep_itr;
            stream_recv buff;

            recv_info(decltype(cluster)::iterator ep_itr, int size)
                : ep_itr(ep_itr), buff(size) {}
        };
        using RecvStreamMap = std::map<uint32_t, recv_info>; // key: 受信ストリームID
        RecvStreamMap recv_stream_map; // 受信ストリーム
        std::shared_mutex recv_stream_mutex; // 受信ストリームを扱うためのmutex
        std::condition_variable_any recv_stream_cv; // 受信ストリームの更新通知管理
        std::optional<uint32_t> prev_recv_streamid; // 直前にチェックした受信ストリームID
    };

}
