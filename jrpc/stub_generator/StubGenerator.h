//
// Created by frank on 18-1-23.
//

#ifndef JRPC_STUBGENERATOR_H
#define JRPC_STUBGENERATOR_H

#include <jackson/Value.h>

namespace jrpc
{

class StubGenerator
{
protected:
    explicit
    StubGenerator(json::Value& proto):
            serviceInfo_(parseProto(proto))
    {}

    virtual std::string generate() = 0;

protected:

    struct RpcReturn
    {
        std::string name;
        json::Value params;
        json::Value returns;
    };

    struct RpcNotify
    {
        std::string name;
        json::Value params;
    };

    struct ServiceInfo
    {
        std::string name;
        std::vector<RpcReturn> rpcReturn;
        std::vector<RpcNotify> rpcNotify;
    };

    ServiceInfo serviceInfo_;

private:
    ServiceInfo parseProto(json::Value& proto);
};

}

#endif //JRPC_STUBGENERATOR_H
