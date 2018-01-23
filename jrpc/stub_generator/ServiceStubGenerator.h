//
// Created by frank on 18-1-23.
//

#ifndef JRPC_SERVICESTUBGENERATOR_H
#define JRPC_SERVICESTUBGENERATOR_H

#include <unordered_map>

#include <jrpc/stub_generator/StubGenerator.h>

namespace jrpc
{

class ServiceStubGenerator: public StubGenerator
{
public:
    ServiceStubGenerator(json::Value& proto):
            StubGenerator(proto)
    {}

    std::string genServiceStub();

private:
    std::string genMacroName();
    std::string genUserClassName();
    std::string genStubClassName();
    std::string genStubProcedureBindings();
    std::string genStubProcedureDefinitions();

    std::string genStubProcedureName(const std::string& procedureName);
    std::string genProcedureParams(const std::string& procedureName);

    std::string genParamsFromJsonArray();
    std::string genParamsFromJsonObject();
};

}



#endif //JRPC_SERVICESTUBGENERATOR_H
