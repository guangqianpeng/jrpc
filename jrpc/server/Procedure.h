//
// Created by frank on 17-12-31.
//

#ifndef JRPC_PROCEDURE_H
#define JRPC_PROCEDURE_H


#include <jackson/Value.h>

#include <jrpc/util.h>

namespace jrpc
{

typedef std::function<void(json::Value&, json::Value&)> ProcedureReturnCallback;
typedef std::function<void(json::Value&)> ProcedureNotifyCallback;

template <typename Func>
class Procedure: noncopyable
{
public:
    template<typename... ParamNameAndTypes>
    Procedure(Func&& callback, ParamNameAndTypes &&... nameAndTypes):
            callback_(std::forward<Func>(callback))
    {
        constexpr int n = sizeof...(nameAndTypes);
        static_assert(n % 2 == 0, "procedure must have param name and type pairs");

        if constexpr (n > 0)
            initProcedure(nameAndTypes...);
    }

    // procedure call
    void invoke(json::Value& request, json::Value& response);
    // procedure notify
    void invoke(json::Value& request);

private:
    template<typename Name, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, json::ValueType parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same<Name, const char *>::value ||
                      std::is_same<Name, std::string_view>::value,
                      "param name must be 'const char*' or 'std::string_view'");
        params_.emplace_back(paramName, parmType);
        if constexpr (sizeof...(ParamNameAndTypes) > 0)
            initProcedure(nameAndTypes...);
    }

    template<typename Name, typename Type, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, Type parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same<Type, json::ValueType>::value, "param type must be json::ValueType");
    }

    void validateRequest(json::Value& request) const;
    bool validateGeneric(json::Value& request) const;

private:
    struct Param
    {
        Param(std::string_view paramName_, json::ValueType paramType_) :
                paramName(paramName_),
                paramType(paramType_)
        {}

        std::string_view paramName;
        json::ValueType paramType;
    };

private:
    Func callback_;
    std::vector<Param> params_;
};


typedef Procedure<ProcedureReturnCallback> ProcedureReturn;
typedef Procedure<ProcedureNotifyCallback> ProcedureNotify;

}

#endif //JRPC_PROCEDURE_H
