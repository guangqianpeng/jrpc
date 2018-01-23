//
// Created by frank on 18-1-23.
//

#include "ServiceStubGenerator.h"

using namespace jrpc;

namespace
{

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    while (true) {
        size_t i = str.find(from);
        if (i != std::string::npos) {
            str.replace(i, from.size(), to);
        }
        else return;
    }
}

std::string serviceStubTemplate(
        const std::string& macroName,
        const std::string& userClassName,
        const std::string& stubClassName,
        const std::string& stubProcedureBindings,
        const std::string& stubProcedureDefinitions)
{
    std::string str = R"(
#ifndef JRPC_[macroName]_H
#define JRPC_[macroName]_H

#include <jackson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcServer.h>
#include <jrpc/server/RpcService.h>

class [userClassName];

namespace jrpc
{


template <typename S>
class [stubClassName]: noncopyable
{
protected:
    explicit
    [stubClassName](RpcServer& server)
    {
        static_assert(std::is_same_v<S, [userClassName]>,
                      "derived class name should be '[userClassName]'");

        auto service = new RpcService;

        [stubProcedureBindings]

        server.addService("Hello", service);
    }

    ~[stubClassName]() = default;

private:
    [stubProcedureDefinitions]

private:
    S& convert()
    {
        return static_cast<S&>(*this);
    }
};

}

#endif //JRPC_[macro_name]_H
)";
    replaceAll(str, "[macroName]", macroName);
    replaceAll(str, "[userClassName]", userClassName);
    replaceAll(str, "[stubClassName]", stubClassName);
    replaceAll(str, "[stubProcedureBindings]", stubProcedureBindings);
    replaceAll(str, "[stubProcedureDefinitions]", stubProcedureDefinitions);
    return str;
}

std::string stubProcedureBindTemplate(
        const std::string& procedureName,
        const std::string& stubClassName,
        const std::string& stubProcedureName,
        const std::string& procedureParams)
{
    std::string str = R"(
service->addProcedureReturn("[procedureName]", new ProcedureReturn(
        std::bind(&[stubClassName]::[stubProcedureName], this, _1, _2),
        [procedureParams]
));
)";
    replaceAll(str, "[procedureName]", procedureName);
    replaceAll(str, "[stubClassName]", stubClassName);
    replaceAll(str, "[stubProcedureName]", stubProcedureName);
    replaceAll(str, "[procedureParams]", procedureParams);
    return str;
}

std::string stubProcedureDefineTemplate(
        const std::string& paramsFromJsonArray,
        const std::string& paramsFromJsonObject,
        const std::string& procedureName)
{
   std::string str = R"(
void HelloStub(json::Value& request, const RpcDoneCallback& done)
{
    auto& params = request["params"];

    std::string user;
    if (params.isArray()) {
        [paramsFromJsonArray]
    }
    else {
        [paramsFromJsonObject]
    }

    // request is moved in
    convert().[procedureName](user, UserDoneCallback(request, done));
}
)";
    replaceAll(str, "[paramsFromJsonArray]", paramsFromJsonArray);
    replaceAll(str, "[paramsFromJsonObject]", paramsFromJsonObject);
    replaceAll(str, "[procedureName]", procedureName);
    return str;
}

}


std::string ServiceStubGenerator::genServiceStub()
{
    auto macroName = genMacroName();
    auto userClassName = genUserClassName();
    auto stubClassName = genStubClassName();
    auto bindings = genStubProcedureBindings();
    auto definitions = genStubProcedureDefinitions();
    return serviceStubTemplate(macroName,
                               userClassName,
                               stubClassName,
                               bindings,
                               definitions);
}

std::string ServiceStubGenerator::genMacroName()
{
    std::string result = serviceInfo_.name;
    for (char& c: result)
        c = static_cast<char>(toupper(c));
    return result.append("SERVICESTUB");
}

std::string ServiceStubGenerator::genUserClassName()
{
    return serviceInfo_.name + "Service";
}

std::string ServiceStubGenerator::genStubClassName()
{
    return serviceInfo_.name + "ServiceStub";
}

std::string ServiceStubGenerator::genStubProcedureBindings()
{
    std::string result;
    for (auto& p: serviceInfo_.rpcReturn) {
        auto procedureName = p.name;
        auto stubClassName = genStubClassName();
        auto stubProcedureName = genStubProcedureName(procedureName);
        auto procedureParams = genProcedureParams(procedureName);

        auto binding = stubProcedureBindTemplate(
                procedureName,
                stubClassName,
                stubProcedureName,
                procedureParams);
        result.append(binding);
        result.append("\n");
    }
    return result;
}


std::string ServiceStubGenerator::genStubProcedureDefinitions()
{
    std::string result;
    for (auto& p: serviceInfo_.rpcNotify) {
        auto paramsFromJsonArray = genParamsFromJsonArray();
        auto paramsFromJsonObject = genParamsFromJsonObject();
        auto procedureName = p.name;
        auto define = stubProcedureDefineTemplate(
                paramsFromJsonArray,
                paramsFromJsonObject,
                procedureName);
        result.append(define);
        result.append("\n");
    }
    return result;
}

std::string genStubProcedureName(const std::string& procedureName)
{
    return procedureName + "Stub";
}

std::string ServiceStubGenerator::genProcedureParams(const std::string& procedureName)
{

}

std::string ServiceStubGenerator::genParamsFromJsonArray()
{

}

std::string ServiceStubGenerator::genParamsFromJsonObject()
{

}