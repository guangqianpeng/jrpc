//
// Created by frank on 17-12-31.
//

#ifndef JRPC_SERVERSTUB_H
#define JRPC_SERVERSTUB_H


#include <jackson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcServer.h>
#include <jrpc/server/RpcService.h>

class HelloService;

namespace jrpc
{

// 75\r\n{"jsonrpc": "2.0", "method": "Hello.Hello", "params": ["frank"], "id": 1}\r\n
// 77\r\n[{"jsonrpc": "2.0", "method": "Hello.Hello", "params": ["frank"], "id": 1},2]\r\n
// 86\r\n[{"jsonrpc": "2.0", "method": "Hello.Add", "params": {"lhs": 1, "rhs": 2}, "id": 1}]\r\n
// 47\r\n{"jsonrpc": "2.0", "method": "Hello.Goodbye"}\r\n

class HelloDoneCallback
{
public:
    HelloDoneCallback(json::Value& request, const RpcDoneCallback& callback)
            : request_(request),
              callback_(callback)
    {}

    void operator()(std::string result) const
    {
        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("id", request_["id"]);
        response.addMember("result", result);
        callback_(response);
    }

private:
    mutable json::Value request_;
    RpcDoneCallback callback_;
};

class AddDoneCallback
{
public:
    AddDoneCallback(json::Value& request, const RpcDoneCallback& callback)
            : request_(request),
              callback_(callback)
    {}

    void operator()(int32_t result) const
    {
        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("id", request_["id"]);
        response.addMember("result", result);
        callback_(response);
    }

private:
    mutable json::Value request_;
    RpcDoneCallback callback_;
};

template <typename S>
class HelloServiceStub: noncopyable
{
protected:
    explicit
    HelloServiceStub(RpcServer& server)
    {
        static_assert(std::is_same_v<S, HelloService>,
                      "derived class name should be 'HelloServer'");

        auto service = new RpcService;

        // add procedure return call
        service->addProcedureReturn("Hello", new ProcedureReturn(
                std::bind(&HelloServiceStub::HelloStub, this, _1, _2),
                "user", json::TYPE_STRING
        ));
        service->addProcedureReturn("Add", new ProcedureReturn(
                std::bind(&HelloServiceStub::AddStub, this, _1, _2),
                "lhs", json::TYPE_INT32,
                "rhs", json::TYPE_INT32
        ));
        // add notification
        service->addProcedureNotify("Goodbye", new ProcedureNotify(
                std::bind(&HelloServiceStub::GoodbyeStub, this)
        ));
        // register service
        server.addService("Hello", service);
    }
    ~HelloServiceStub() = default;

private:
    void HelloStub(json::Value& request, const RpcDoneCallback& done)
    {
        auto& params = request["params"];

        auto&& user = [&params]()->json::Value& {
            if (params.isArray())
                return params[0];
            return params["user"];
        }();

        // request is moved in
        convert().Hello(user.getString(), HelloDoneCallback(request, done));
    }

    void AddStub(json::Value& request, const RpcDoneCallback& done)
    {
        auto& params = request["params"];

        int32_t lhs, rhs;
        if (params.isArray()) {
            lhs = params[0].getInt32();
            rhs = params[1].getInt32();
        }
        else {
            assert(params.isObject());
            lhs = params["lhs"].getInt32();
            rhs = params["rhs"].getInt32();
        }

        convert().Add(lhs, rhs, AddDoneCallback(request, done));
    }

    void GoodbyeStub()
    {
        convert().Goodbye();
    }


private:
    S& convert()
    {
        return static_cast<S&>(*this);
    }
};

}

#endif //JRPC_SERVERSTUB_H
