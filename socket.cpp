#include "socket.hpp"

gcom::socket::socket(uint16_t port)
    : system_state(ATOMIC_FLAG_INIT)
{
    // UDPソケットを作成する
    sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        throw std::exception();
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        throw std::exception();
    }

    // epollインスタンスを作成する
    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        throw std::exception();
    }

    // epollインスタンスにUDPソケットを登録する
    register_epoll_event(sockfd);

    // マルチキャストstream idを生成し、それぞれの送信バッファを作成する
    for (int stream_cnt = 0; stream_cnt < num_multicast_streams; stream_cnt++)
    {
        // stream idを生成
        int streamid = get_streamid();

        // stream_send_mapに登録
        multicast_stream_map.emplace(
            std::piecewise_construct,
            std::make_tuple(streamid),
            std::make_tuple(stream_buffer_size)
        );
    }

    // フラグを起動中に設定する
    system_state.test_and_set();

    // バックグラウンドスレッドを起動する
    bgthread = std::thread([this]{ background(); });
}

gcom::socket::~socket()
{
    // フラグを停止中に設定する
    system_state.clear();

    // バックグラウンドスレッドをjoinする
    if (bgthread.joinable())
    {
        bgthread.join();
    }

    ::close(epollfd);
    ::close(sockfd);
}

// void gcom::socket::send_to(const void *buf, size_t len, endpoint& dest)
// {
//     // 宛先をclusterから検索する
//     decltype(cluster)::iterator ep_itr = cluster.find(dest);
//     if (ep_itr  == cluster.end())
//     {
//         // clusterに登録されていない場合は送信しない
//         throw std::exception();
//     }

//     // ストリームを確定する
//     int streamid;
//     decltype(unicast_stream_send_map)::iterator itr = unicast_stream_send_map.find(streamid);

//     // バッファに格納する
//     itr->second.body.push_packets((unsigned char *)buf, len, payload_size);
// }

void gcom::socket::send_all(const void *buf, size_t len)
{
    int streamid; // 挿入するストリームID
    uint32_t remaining; // ストリームIDが示すストリームの空き容量

    // 空き容量が大きいストリームを選択する
    streamid = multicast_stream_map.begin()->first;
    remaining = multicast_stream_map.begin()->second.buff.get_remaining_size();
    for (auto itr = multicast_stream_map.begin();
            itr != multicast_stream_map.end();
            itr++)
    {
        if (remaining < itr->second.buff.get_remaining_size())
        {
            streamid = itr->first;
            remaining = itr->second.buff.get_remaining_size();
        }
    }

    // 送信バッファサイズの不足
    if (remaining < len)
    {
        throw std::exception();
    }

    // 選択したストリームのバッファに格納する
    multicast_stream_map.at(streamid)
                        .buff
                        .push_packets((unsigned char *)buf, len, payload_size);
}

uint32_t gcom::socket::recv_from(void *buf, endpoint *from)
{
    uint32_t len;
    decltype(recv_stream_map)::iterator itr;

    // 最初に確認するストリームを選択する
    {
        std::shared_lock<std::shared_mutex> lock(recv_stream_mutex);

        // ストリームが1つもない場合は通知されるまで待機する
        recv_stream_cv.wait(lock, [this] { return !recv_stream_map.empty(); });

        if (!prev_recv_streamid.has_value()) 
        {
            itr = recv_stream_map.begin();
        }
        else
        {
            itr = recv_stream_map.find(*prev_recv_streamid);
            itr++;
            if (itr == recv_stream_map.end()) {
                itr = recv_stream_map.begin();
            }
        }
    }

    // ラウンドロビン形式でストリームを探索する
    while (true) {
        std::shared_lock<std::shared_mutex> lock(recv_stream_mutex);

        len = itr->second.buff.try_pop_packets(static_cast<unsigned char*>(buf));
        if (len > 0) {
            *from = *(itr->second.ep_itr);
            prev_recv_streamid = itr->first;
            return len;
        }

        ++itr;
        if (itr == recv_stream_map.end()) {
            itr = recv_stream_map.begin();
        }

        // どのストリームにもデータがなかったので通知を待つ
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_for_querying_recv_buffer));
    }
}

void gcom::socket::add_group(endpoint &ep)
{
    cluster.insert(ep);

    // // 追加する宛先へのユニキャストstream idを生成し、その送信バッファを作成する
    // for (int stream_cnt = 0; stream_cnt < num_unicast_streams; stream_cnt++)
    // {
    //     // stream idを生成
    //     int streamid = get_streamid();

    //     unicast_streamid.insert();

    //     // stream_send_mapに登録
    //     unicast_stream_map.insert({streamid, {}});
    // }
}

void gcom::socket::background()
{
    struct epoll_event events[max_epoll_events];
    int nfds;

    while (system_state.test())
    {
        nfds = epoll_wait(epollfd, events, max_epoll_events, epoll_wait_timeout);
        if (nfds > 0)
        {
            for (int i = 0; i < nfds; i++)
            {
                // sockfdのイベントが発生した（他ノードからデータを受信した）場合
                if (events[i].data.fd == sockfd)
                {
                    handle_arriving_packet();
                    continue;
                }

                // 再送タイムアウト
            }
        }

        // 定期的（最大epoll_wait_timeout間隔）で送信バッファ内のデータを送信する
        transmit_ready_packets();
    }
    std::cout << "fin background()state: " << system_state.test() << std::endl;
}

void gcom::socket::handle_arriving_packet()
{
    std::cout << "handle_arriving_packet" << std::endl;
    unsigned char payload[payload_size];
    header hdr;

    // UDPソケットから入力する
    std::pair<int, endpoint> result_input_packet = input_packet(&hdr, payload);

    // // NACKパケットを受信した場合
    // if (hdr.flag & (1<<(int)(header_flag::FLG_CTL_NCK)))
    // {

    // }
    // マルチキャストデータパケットを受信した場合
    if (hdr.flag & (1<<(int)(header_flag::FLG_DAT_MLT_SRC)))
    {
        {
            std::unique_lock<std::shared_mutex> lock(recv_stream_mutex);

            auto stream_itr = recv_stream_map.find(hdr.streamid);
            
            // 新しいストリームIDを受信した場合
            if (stream_itr == recv_stream_map.end())
            {
                decltype(cluster)::iterator ep_itr = cluster.find(result_input_packet.second);

                // 登録されていないノードから受信した場合
                if (ep_itr == cluster.end())
                {
                    throw std::exception();
                }
                
                auto result_emplace = recv_stream_map.emplace(
                    std::piecewise_construct,
                    std::make_tuple(hdr.streamid),
                    std::make_tuple(ep_itr, stream_buffer_size)
                );
                stream_itr = result_emplace.first;
            }
            
            stream_itr->second.buff.insert_packet(payload, result_input_packet.first, hdr.seq, hdr.head, hdr.tail); 
            std::cout << "insert" << std::endl;
        }

        recv_stream_cv.notify_all();
        return;
    }
    // // マルチキャストデータパケットをリカバリーノードから受信した場合
    // if (hdr.flag & (1<<(int)(header_flag::FLG_DAT_MLT_RCV)))
    // {

    // }
}

void gcom::socket::transmit_ready_packets()
{
    uint32_t seq;
    unsigned char payload[payload_size];
    uint32_t total_len, payload_len;

    // マルチキャストストリーム内のデータを出力
    for (auto stream_itr = multicast_stream_map.begin();
         stream_itr != multicast_stream_map.end();
         stream_itr++)
    {
        // ストリームバッファ内に送信可能データが格納されている場合
        if (stream_itr->second.buff.has_ready_packets(&seq, &total_len))
        {
            header hdr;
            hdr.flag = 1<<(int)(header_flag::FLG_DAT_MLT_SRC);
            hdr.streamid = stream_itr->first;
            hdr.seq = seq;
            payload_len = stream_itr->second.buff.get_packet(hdr.seq, payload, &(hdr.head), &(hdr.tail));

            // 登録されているノードにパケットを送信する
            for (auto to_itr = cluster.begin();
                to_itr != cluster.end();
                to_itr++)
            {
                output_packet(&hdr, payload, payload_len, *to_itr);
            }
        }
    }

    // ユニキャストストリーム内のデータを出力
}

void gcom::socket::output_packet(const header* hdr, const void *payload, size_t payload_len, endpoint to_ep)
{
    struct sockaddr_in to_addr;
    unsigned char buf[packet_size];
    uint32_t len = 0; // パケットサイズ + ペイロードサイズ
    uint32_t output_len;

    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = inet_addr(to_ep.get_ipaddr().c_str());
    to_addr.sin_port = htons(to_ep.get_port());

    memcpy(buf, hdr, sizeof(header));
    len += sizeof(header);
    memcpy(buf + sizeof(header), payload, payload_len);
    len += payload_len;

    output_len = ::sendto(sockfd, buf, len, 0, (struct sockaddr *)&(to_addr), sizeof(to_addr));
    if (output_len != len)
    {
        throw std::exception();
    }

    fprintf(stderr,
            "output, flag:%d, streamid:%" PRIu32 ", seq:%" PRIu32 ", head:%" PRIu32 ", tail:%" PRIu32 ", %s\n",
            hdr->flag, hdr->streamid, hdr->seq, hdr->head, hdr->tail, (char*)payload);
#ifdef OUTPUT_PACKET_LOG
    fprintf(stderr, "%s:%d output_packet(): output_len:%ld\n", __FILE__, __LINE__, output_len);
#endif
}

std::pair<size_t, gcom::endpoint> gcom::socket::input_packet(header* hdr, void *payload)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    unsigned char buf[packet_size];
    ssize_t input_len; // パケットサイズ + ペイロードサイズ
    ssize_t payload_len; // ペイロードサイズ

    input_len = ::recvfrom(sockfd, buf, packet_size, 0, (struct sockaddr *)&addr, &addr_len);
    if (input_len == -1) // error
    {
        throw std::exception();
    }
    else if (input_len < (ssize_t)sizeof(header))
    {
        throw std::exception();
    }

    memcpy(hdr, buf, sizeof(header));
    payload_len = input_len - sizeof(header);
    memcpy(payload, buf + sizeof(header), payload_len);

    endpoint from(inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), false);

    fprintf(stderr,
        "input, flag:%d, streamid:%" PRIu32 ", seq:%" PRIu32 ", head:%" PRIu32 ", tail:%" PRIu32 ", %s\n",
        hdr->flag, hdr->streamid, hdr->seq, hdr->head, hdr->tail, (char*)payload);
#ifdef OUTPUT_PACKET_LOG
    fprintf(stderr,
            "%s:%d input_packet(): input_len:%ld hdr.flag:%ld\n",
            __FILE__, __LINE__, input_len, hdr->flag);
#endif

    return std::make_pair(payload_len, from);
}

void gcom::socket::register_epoll_event(int fd)
{
    struct epoll_event ev;

    // 指定されたfdをepollfdに登録する
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        throw std::exception();
    }
}

uint32_t gcom::socket::get_streamid()
{
    std::mt19937 gen(std::random_device{}());
    // 1~UINT32_MAXの値をランダム生成する
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
    return dist(gen);
}
