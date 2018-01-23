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
        throw RequestException(e.err(), id, e.detail());
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
json::Value& findValue(json::Value& request, json::Value& id, const char *key)
{
    // wrap exception
    try {
        return findValue<types...>(request, key);
    }
    catch (RequestException &e) {
        throw RequestException(e.err(), id, e.detail());
    }
}

bool isNotify(const json::Value& request)
{
    return request.findMember("id") == request.memberEnd();
}

bool hasParams(const json::Value& request)
{
    return request.findMember("params") != request.memberEnd();
}

// a thread safe batch response container
// use shared_ptr to manage json value
class ThreadSafeBatchResponse
{
public:
    explicit
    ThreadSafeBatchResponse(const RpcDoneCallback& done):
            data_(std::make_shared<ThreadSafeData>(done))
    {}

    void addResponse(json::Value response)
    {
        std::unique_lock lock(data_->mutex);
        data_->responses.addValue(response);
    }

private:
    struct ThreadSafeData
    {
        explicit
        ThreadSafeData(const RpcDoneCallback& done_):
                responses(json::TYPE_ARRAY),
                done(done_)
        {}

        ~ThreadSafeData()
        {
            // last reference to data is destructing, so notify RPC server we are done
            done(responses);
        }

        std::mutex mutex;
        json::Value responses;
        RpcDoneCallback done;
    };

    typedef std::shared_ptr<ThreadSafeData> DataPtr;
    DataPtr data_;
};

}

void RpcServer::addService(std::string_view serviceName, RpcService *service)
{
    assert(services_.find(serviceName) == services_.end());
    services_.emplace(serviceName, service);
}

void RpcServer::handleRequest(const std::string& json,
                              const RpcDoneCallback& done)
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
                handleSingleRequest(request, done);
            break;
        case json::TYPE_ARRAY:
            handleBatchRequests(request, done);
            break;
        default:
            throw RequestException(RPC_INVALID_REQUEST, "request should be json object or array");
    }
}

void RpcServer::handleSingleRequest(json::Value& request,
                                    const RpcDoneCallback& done)
{
    validateRequest(request);

    auto& id = request["id"];
    auto methodName = request["method"].getStringView();
    auto pos = methodName.find('.');
    if (pos == std::string_view::npos)
        throw RequestException(RPC_INVALID_REQUEST, id, "missing service name in method");

    auto serviceName = methodName.substr(0, pos);
    auto it = services_.find(serviceName);
    if (it == services_.end())
        throw RequestException(RPC_METHOD_NOT_FOUND, id, "service not found");

    // skip service name and '.'
    methodName.remove_prefix(pos + 1);
    if (methodName.length() == 0)
        throw RequestException(RPC_INVALID_REQUEST, id, "missing method name in method");

    auto& service = it->second;
    service->callProcedureReturn(methodName, request, done);
}

void RpcServer::handleBatchRequests(json::Value& requests,
                                    const RpcDoneCallback& done)
{
    size_t num = requests.getSize();
    if (num == 0)
        throw RequestException(RPC_INVALID_REQUEST, "batch request is empty");

    ThreadSafeBatchResponse responses(done);

    try {
        size_t n = requests.getSize();
        for (size_t i = 0; i < n; i++) {

            auto& request = requests[i];

            if (!request.isObject()) {
                throw RequestException(RPC_INVALID_REQUEST, "request should be json object");
            }

            if (isNotify(request)) {
                handleSingleNotify(request);
            }
            else {
                handleSingleRequest(request, [=](json::Value response) mutable {
                    responses.addResponse(response);
                });
            }
        }
    }
    catch (RequestException &e) {
        auto response = wrapException(e);
        responses.addResponse(response);
    }
    catch (NotifyException &e) {
        // todo: print something here
    }
}

void RpcServer::handleSingleNotify(json::Value& request)
{
    validateNotify(request);

    auto methodName = request["method"].getStringView();
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

void RpcServer::validateRequest(json::Value& request)
{
    auto& id = findValue<
            json::TYPE_STRING,
            json::TYPE_NULL, // fixme: null id is evil
            json::TYPE_INT32,
            json::TYPE_INT64>(request, "id");

    auto& version = findValue<json::TYPE_STRING>(request, id, "jsonrpc");
    if (version.getStringView() != "2.0")
        throw RequestException(RPC_INVALID_REQUEST,
                               id, "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, id, "method");
    if (method.getStringView() == "rpc.") // internal use
        throw RequestException(RPC_METHOD_NOT_FOUND,
                               id, "method name is internal use");

    // jsonrpc, method, id, params
    size_t nMembers = 3u + hasParams(request);

    if (request.getSize() != nMembers)
        throw RequestException(RPC_INVALID_REQUEST, id, "unexpected field");
}

void RpcServer::validateNotify(json::Value& request)
{
    auto& version = findValue<json::TYPE_STRING>(request, "jsonrpc");
    if (version.getStringView() != "2.0")
        throw NotifyException(RPC_INVALID_REQUEST, "jsonrpc version is unknown/unsupported");

    auto& method = findValue<json::TYPE_STRING>(request, "method");
    if (method.getStringView() == "rpc.") // internal use
        throw NotifyException(RPC_METHOD_NOT_FOUND, "method name is internal use");

    // jsonrpc, method, params, no id
    size_t nMembers = 2u + hasParams(request);

    if (request.getSize() != nMembers)
        throw NotifyException(RPC_INVALID_REQUEST, "unexpected field");
}
