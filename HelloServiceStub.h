//
// Created by frank on 17-12-31.
//

#ifndef JRPC_HELLOSERVICESTUB_H
#define JRPC_HELLOSERVICESTUB_H


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

        std::string user;
        if (params.isArray()) {
            user = params[0].getString();
        }
        else {
            user = params["user"].getString();
        }

        // request is moved in
        convert().Hello(user, UserDoneCallback(request, done));
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

        convert().Add(lhs, rhs, UserDoneCallback(request, done));
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

#endif //JRPC_HELLOSERVICESTUB_H
