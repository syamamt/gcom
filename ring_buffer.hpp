#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <cstring>
#include <algorithm>

namespace gcom
{

    template<typename T>
    class ring_buffer
    {
    public:
        ring_buffer(T size) : size(size), write_idx(0), read_idx(0), zombie_idx(0)
        {
            body = new unsigned char[size];
        }

        // 指定したサイズ分だけ領域を確保し、そこデータを書き込む
        // 書き込んだ先頭のインデックス（直前のWrite Index）を返す
        T push(unsigned char *data, T len)
        {
            T offset;
            T prev_write_idx;
        
            if (len > size - (write_idx - read_idx))
            {
                throw std::exception();
            }
        
            offset = write_idx % size;
            if (offset + len > size)
            {
                std::memcpy((void *)(body + offset), data, size);
                std::memcpy((void *)body, data + (size - offset), len - (size - offset));
            }
            else
            {
                std::memcpy((void *)(body + offset), data, len);
            }
        
        
            prev_write_idx = write_idx;
            write_idx += len;
        
            if (write_idx - zombie_idx > size)
            {
                // calc zombie index
                zombie_idx = write_idx - size;
            }
        
            return prev_write_idx;
        }

        // 指定したサイズ分だけ領域を確保する
        T push_empty(T len)
        {
            T prev_write_idx;
        
            if (len > size - (write_idx - read_idx))
            {
                throw std::exception();
            }
            
            prev_write_idx = write_idx;
            write_idx += len;
        
            if (write_idx - zombie_idx > size)
            {
                // calc zombie index
                zombie_idx = write_idx - size;
            }
        
            return prev_write_idx;
        }

        // 指定したサイズ分だけ領域を解放する
        // 最も小さい有効なインデックス（新しいRead Index）を返す
        T pop(T len)
        {
            if (len > write_idx - read_idx)
            {
                throw std::exception();
            }
        
            read_idx += len;
            return read_idx;
        }

        // 指定したインデックスにデータを書き込む
        // 指定されたインデックスの領域が有効でない場合、何もしない
        void set(T idx, unsigned char *data, T len)
        {
            T offset;
        
            if (idx < read_idx || idx + len > write_idx)
            {
                throw std::exception();
            }
        
            offset = idx % size;
            if (offset + len > size)
            {
                std::memcpy((void *)(body + offset), data, size);
                std::memcpy((void *)body, data + (size - offset), len - (size - offset));
            }
            else
            {
                std::memcpy((void *)(body + offset), data, len);
            }
        }

        // 指定したインデックスのデータを読み込む
        // 指定されたインデックスの領域が有効でない場合、何もしない
        void get(T idx, unsigned char *data, T len)
        {
            T offset;
        
            if (idx < zombie_idx || idx > write_idx - len)
            {
                throw std::exception();
            }
        
            offset = idx % size;
            if (offset + len > size)
            {
                std::memcpy((void *)data, body + offset, size - offset);
                std::memcpy((void *)(data + (size - offset)), body, len - (size - offset));
            }
            else
            {
                std::memcpy((void *)data, body + offset, len);
            }
        }

        T get_write_idx()
        {
            return write_idx;
        }

        T get_read_idx()
        {
            return read_idx;
        }

        T get_zombie_idx()
        {
            return zombie_idx;
        }

        T get_size()
        {
            return size;
        }
    private:
        unsigned char *body;

        // バッファサイズ
        T size;

        // 
        T write_idx;

        //
        T read_idx;

        //
        T zombie_idx;
    };

} // namespace gcom
