//
// Created by frank on 17-12-30.
//

#include <jackson/Document.h>

#include <jrpc/Exception.h>
#include <jrpc/server/RpcService.h>
#include <jrpc/server/RpcServer.h>

using namespace jrpc;

namespace
{

template <json::ValueType dst, json::ValueType... rest>
void checkValueType(json::ValueType type)
{
    if (dst == type)
        return;
    if constexpr (sizeof...(rest) > 0)
        checkValueType<rest...>(type);
    else
        throw RequestException(RPC_INVALID_REQUEST, "bad type of at least one field");
}

template <json::ValueType... types>
void checkValueType(json::ValueType type, json::Value& id)
{
    // wrap exception
    try {
        checkValueType<types...>(type);
    }
    catch (RequestException& e) {
        throw RequestException(e.err(), std::move(id), e.detail());
    }
}

template <json::ValueType... types>
json::Value& findValue(json::Value &request, const char *key)
{
    static_assert(sizeof...(types) > 0, "expect at least one type");

    auto it = request.findMember(key);
    if (it == request.memberEnd())
        throw RequestException(RPC_INVALID_REQUEST, "missing at least one field");
    checkValueType<types...>(it->value.getType());
    return it->value;
}

template <json::ValueType... types>
json::Value& findValue(json::Value &request, json::Value& id, const char *key)
{
    // wrap exception
    try {
        return findValue<types...>(request, key);
    }
    catch (RequestException &e) {
        throw RequestException(e.err(), std::move(id), e.detail());
    }
}

bool isNotify(json::Value& request)
{
    return request.findMember("id") == request.memberEnd();
}

bool hasParams(json::Value& request)
{
    return request.findMember("params") != request.memberEnd();
}

}

void RpcServer::addService(std::string_view serviceName, RpcService *service)
{
    assert(services_.find(serviceName) == services_.end());
    services_.emplace(serviceName, service);
}

void RpcServer::handleRequest(std::string& json, json::Value& response)
{
    json::Document request;
    json::ParseError err = request.parse(json);
    if (err != json::PARSE_OK)
        throw RequestException(RPC_PARSE_ERROR, json::parseErrorStr(err));

    switch (request.getType()) {
        case json::TYPE_OBJECT:
            if (isNotify(request))
                handleSingleNotify(request);
            else
                handleSingleRequest(request, response);
            break;
        case json::TYPE_ARRAY:
            handleBatchRequests(request, response);
            break;
        default:
            throw RequestException(RPC_INVALID_REQUEST, "request should be json object or array");
    }
}

void RpcServer::handleSingleRequest(json::Value& request, json::Value& response)
{
    validateRequest(request);

    auto& id = request["id"];
    auto methodName = request["method"].getString();
    auto pos = methodName.find('.');
    if (pos == std::string_view::npos)
        throw RequestException(RPC_INVALID_REQUEST, std::move(id), "missing service name in method");

    auto serviceName = methodName.substr(0, pos);
    auto it = services_.find(serviceName);
    if (it == services_.end())
        throw RequestException(RPC_METHOD_NOT_FOUND, std::move(id), "service not found");

    // skip service name and '.'
    methodName.remove_prefix(pos + 1);
    if (methodName.length() == 0)
        throw RequestException(RPC_INVALID_REQUEST, std::move(id), "missing method name in method");

    auto& service = it->second;
    service->callProcedureReturn(methodName, request, response);
}

void RpcServer::handleSingleNotify(json::Value& request)
{
    validateNotify(request);

    auto methodName = request["method"].getString();
    auto pos = methodName.find('.');
    if (pos == std::string_view::npos || pos == 0)
        throw NotifyException(RPC_INVALID_REQUEST, "missing service name in method");

    auto serviceName = methodName.substr(0, pos);
    auto it = services_.find(serviceName);
    if (it == services_.end())
        throw RequestException(RPC_METHOD_NOT_FOUND, "service not found");

    // skip service name and '.'
    methodName.remove_prefix(pos + 1);
    if (methodName.length() == 0)
        throw RequestException(RPC_INVALID_REQUEST, "missing method name in method");

    auto& service = it->second;
    service->callProcedureNotify(methodName, request);
}

void RpcServer::handleBatchRequests(json::Value& requests, json::Value& responses)
{
    size_t num = requests.getSize();
    if (num == 0)
        throw RequestException(RPC_INVALID_REQUEST, "batch request is empty");

    responses.setArray();
    try {
        for (size_t i = 0; i < requests.getSize(); i++) {
            if (isNotify(requests[i])) {
                handleSingleNotify(requests[i]);
            }
            else {
                json::Value response;
                handleSingleRequest(requests[i], response);
                responses.addValue(std::move(response));
            }
        }
    }
    catch (RequestException& e) {
        json::Value response;
        wrapException(response, e);
        responses.addValue(std::move(response));
    }
    catch (NotifyException& e) {
        // todo: print something here
    }
}

void RpcServer::validateRequest(json::Value &request)
{
    auto& id = findValue<
            json::TYPE_STRING,
            json::TYPE_NULL, // fixme: null id is evil
            json::TYPE_INT32,
            json::TYPE_INT64>(request, "id");

    auto& version = findValue<json::TYPE_STRING>(request, id, "jsonrpc");
    if (version.getString() != "2.0")
        throw RequestException(RPC_INVALID_REQUEST,
                               std::move(id), "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, id, "method");
    if (method.getString() == "rpc.") // internal use
        throw RequestException(RPC_METHOD_NOT_FOUND,
                               std::move(id), "method name is internal use");

    // jsonrpc, method, id, params
    size_t nMembers = 3u + hasParams(request);

    if (request.getSize() != nMembers)
        throw RequestException(RPC_INVALID_REQUEST, std::move(id), "unexpected field");
}

void RpcServer::validateNotify(json::Value& request)
{
    auto& version = findValue<json::TYPE_STRING>(request, "jsonrpc");
    if (version.getString() != "2.0")
        throw NotifyException(RPC_INVALID_REQUEST, "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, "method");
    if (method.getString() == "rpc.") // internal use
        throw NotifyException(RPC_METHOD_NOT_FOUND, "method name is internal use");

    // jsonrpc, method, params, no id
    size_t nMembers = 2u + hasParams(request);

    if (request.getSize() != nMembers)
        throw NotifyException(RPC_INVALID_REQUEST, "unexpected field");
}
