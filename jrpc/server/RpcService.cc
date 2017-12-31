//
// Created by frank on 17-12-30.
//

#include <jrpc/Exception.h>
#include <jrpc/server/RpcService.h>

using namespace jrpc;

void RpcService::callProcedureReturn(std::string_view methodName, json::Value& request, json::Value& response)
{
    auto it = procedureReturn_.find(methodName);
    if (it == procedureReturn_.end()) {
        throw RequestException(RPC_METHOD_NOT_FOUND,
                               std::move(request["id"]),
                               "method not found");
    }
    it->second->invoke(request, response);
};

void RpcService::callProcedureNotify(std::string_view methodName, json::Value& request)
{
    auto it = procedureNotfiy_.find(methodName);
    if (it == procedureNotfiy_.end()) {
        throw NotifyException(RPC_METHOD_NOT_FOUND,
                              "method not found");
    }
    it->second->invoke(request);
};