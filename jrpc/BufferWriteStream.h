//
// Created by frank on 17-12-29.
//

#ifndef JRPC_BUFFERWRITER_H
#define JRPC_BUFFERWRITER_H

#include <jrpc/util.h>

namespace jrpc
{


class BufferWriteStream: noncopyable
{
public:
    void put(char c)
    {
        buffer_.appendInt8(c);
    }

    void put(std::string_view str)
    {
        buffer_.append(str);
    }

    tinyev::Buffer& get()
    {
        return buffer_;
    }

private:
    tinyev::Buffer buffer_;
};


}


#endif //JRPC_BUFFERWRITER_H
