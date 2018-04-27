//
// Created by frank on 18-1-23.
//

#include <unordered_set>

#include <jrpc/stub/StubGenerator.h>
#include <jrpc/Exception.h>

using namespace jrpc;

namespace
{

void expect(bool result, const char* errMsg)
{
    if (!result) {
        throw StubException(errMsg);
    }
}

}

void StubGenerator::parseProto(json::Value& proto)
{
    expect(proto.isObject(),
           "expect object");
    expect(proto.getSize() == 2,
           "expect 'name' and 'rpc' fields in object");

    auto name = proto.findMember("name");

    expect(name != proto.memberEnd(),
           "missing service name");
    expect(name->value.isString(),
           "service name must be string");
    serviceInfo_.name = name->value.getString();

    auto rpc = proto.findMember("rpc");
    expect(rpc != proto.memberEnd(),
           "missing service rpc definition");
    expect(rpc->value.isArray(),
           "rpc field must be array");

    size_t n = rpc->value.getSize();
    for (size_t i = 0; i < n; i++) {
        parseRpc(rpc->value[i]);
    }
}

void StubGenerator::parseRpc(json::Value& rpc)
{
    expect(rpc.isObject(),
           "rpc definition must be object");

    auto name = rpc.findMember("name");
    expect(name != rpc.memberEnd(),
           "missing name in rpc definition");
    expect(name->value.isString(),
           "rpc name must be string");

    auto params = rpc.findMember("params");
    bool hasParams = params != rpc.memberEnd();
    if (hasParams) {
        validateParams(params->value);
    }

    auto returns = rpc.findMember("returns");
    bool hasReturns = returns != rpc.memberEnd();
    if (hasReturns) {
        validateReturns(returns->value);
    }

    auto paramsValue = hasParams ?
                       params->value :
                       json::Value(json::TYPE_OBJECT);

    if (hasReturns) {
        RpcReturn r(name->value.getString(), paramsValue, returns->value);
        serviceInfo_.rpcReturn.push_back(r);
    }
    else {
        RpcNotify r(name->value.getString(), paramsValue);
        serviceInfo_.rpcNotify.push_back(r);
    }
}

void StubGenerator::validateParams(json::Value& params)
{
    std::unordered_set<std::string_view> set;

    for (auto& p: params.getObject()) {

        auto key =  p.key.getStringView();
        auto unique = set.insert(key).second;
        expect(unique, "duplicate param name");

        switch (p.value.getType()) {
            case json::TYPE_NULL:
                expect(false, "bad param type");
                break;
            default:
                break;
        }
    }
}

void StubGenerator::validateReturns(json::Value& returns)
{
    switch (returns.getType()) {
        case json::TYPE_NULL:
        case json::TYPE_ARRAY:
            expect(false, "bad returns type");
            break;
        case json::TYPE_OBJECT:
            validateParams(returns);
            break;
        default:
            break;
    }
}