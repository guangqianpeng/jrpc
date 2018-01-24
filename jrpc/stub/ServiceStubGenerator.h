//
// Created by frank on 18-1-23.
//

#ifndef JRPC_SERVICESTUBGENERATOR_H
#define JRPC_SERVICESTUBGENERATOR_H

#include <jrpc/stub/StubGenerator.h>

namespace jrpc
{

class ServiceStubGenerator: public StubGenerator
{
public:
    explicit
    ServiceStubGenerator(json::Value& proto):
            StubGenerator(proto)
    {}

    std::string genStub() override;
    std::string genStubClassName() override;

private:
    std::string genMacroName();
    std::string genUserClassName();
    std::string genStubProcedureBindings();
    std::string genStubProcedureDefinitions();
    std::string genStubNotifyBindings();
    std::string genStubNotifyDefinitions();

    template <typename Rpc>
    std::string genStubGenericName(const Rpc& r);
    template <typename Rpc>
    std::string genGenericParams(const Rpc& r);
    template <typename Rpc>
    std::string genGenericArgs(const Rpc& r);

    template <typename Rpc>
    std::string genParamsFromJsonArray(const Rpc& r);
    template <typename Rpc>
    std::string genParamsFromJsonObject(const Rpc& r);
};

}



#endif //JRPC_SERVICESTUBGENERATOR_H
