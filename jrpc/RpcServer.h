//
// Created by frank on 17-12-30.
//

#ifndef JRPC_RPCSERVER_H
#define JRPC_RPCSERVER_H

#include <memory>
#include <unordered_map>

#include <jackson/Value.h>

#include <jrpc/common/util.h>
#include <jrpc/RpcService.h>
#include <jrpc/ConnectionManager.h>

namespace jrpc
{


class RpcServer: public ConnectionManager<RpcServer>
{

public:
    RpcServer(EventLoop* loop, const InetAddress& listen):
            ConnectionManager(loop, listen)
    {}
    ~RpcServer(){}

    // used by user stub
    void addService(std::string_view serviceName, RpcService* service);

    // used by connection manager
    void handleRequest(std::string& json, json::Value& response);

private:
    void handleSingleRequest(json::Value& request, json::Value& response);
    void handleSingleNotify(json::Value& request);
    void handleBatchRequests(json::Value& requests, json::Value& responses);

    void validateRequest(json::Value& request);
    void validateNotify(json::Value& request);

private:
    typedef std::unique_ptr<RpcService> RpcServeicPtr;
    typedef std::unordered_map<std::string_view, RpcServeicPtr> ServiceList;

    ServiceList services_;
};

}

#endif //JRPC_RPCSERVER_H
