//
// Created by frank on 17-12-30.
//

#ifndef JRPC_EXCEPTION_H
#define JRPC_EXCEPTION_H

#include <exception>

#include <jackson/Value.h>
#include <jrpc/common/RpcError.h>

namespace jrpc
{

class NotifyException: public std::exception
{
public:
    explicit NotifyException(RpcError err, const char* detail):
            err_(err),
            detail_(detail)
    {}

    const char* what() const noexcept
    {
        return err_.asString();
    }
    RpcError err() const
    {
        return err_;
    }
    const char* detail() const
    {
        return detail_;
    }

private:
    RpcError err_;
    const char* detail_;
};

class RequestException: public std::exception
{
public:
    RequestException(RpcError err, json::Value&& id, const char* detail):
            err_(err),
            id_(new json::Value(std::move(id))),
            detail_(detail)
    {}
    explicit RequestException(RpcError err, const char* detail):
            err_(err),
            id_(new json::Value(json::TYPE_NULL)),
            detail_(detail)
    {}


    const char* what() const noexcept
    {
        return err_.asString();
    }
    RpcError err() const
    {
        return err_;
    }
    json::Value& id()
    {
        return *id_;
    }
    const char* detail() const
    {
        return detail_;
    }

private:
    RpcError err_;
    json::Value* id_;
    const char* detail_;
};

}

#endif //JRPC_EXCEPTION_H
