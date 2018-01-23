//
// Created by frank on 17-12-30.
//

#ifndef JRPC_RPCSERVER_H
#define JRPC_RPCSERVER_H

#include <memory>
#include <unordered_map>

#include <jackson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcService.h>
#include <jrpc/server/BaseServer.h>

namespace jrpc
{


class RpcServer: public BaseServer<RpcServer>
{

public:
    RpcServer(EventLoop* loop, const InetAddress& listen):
            BaseServer(loop, listen)
    {}
    ~RpcServer() = default;

    // used by user stub
    void addService(std::string_view serviceName, RpcService* service);

    // used by connection manager
    void handleRequest(const std::string& json,
                       const RpcDoneCallback& done);

private:
    void handleSingleRequest(json::Value& request,
                             const RpcDoneCallback& done);
    void handleBatchRequests(json::Value& requests,
                             const RpcDoneCallback& done);
    void handleSingleNotify(json::Value& request);

    void validateRequest(json::Value& request);
    void validateNotify(json::Value& request);

private:
    typedef std::unique_ptr<RpcService> RpcServeicPtr;
    typedef std::unordered_map<std::string_view, RpcServeicPtr> ServiceList;

    ServiceList services_;
};

}

#endif //JRPC_RPCSERVER_H
