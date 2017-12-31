//
// Created by frank on 17-12-31.
//

#ifndef JRPC_SERVERSTUB_H
#define JRPC_SERVERSTUB_H


#include <jackson/Value.h>

#include <jrpc/common/util.h>
#include <jrpc/RpcServer.h>
#include <jrpc/RpcService.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class HelloServer;

namespace jrpc
{

// 75 {"jsonrpc": "2.0", "method": "Hello.Hello", "params": ["frank"], "id": 1}\r\n
// 47 {"jsonrpc": "2.0", "method": "Hello.Goodbye"}\r\n

template <typename S>
class HelloServerStub: noncopyable
{
protected:
    explicit HelloServerStub(EventLoop* loop_, const InetAddress listen):
            server_(loop_, listen)
    {
        static_assert(std::is_same<S, HelloServer>::value,
                      "derived class name should be 'HelloServer'");

        auto service = new RpcService;

        // add procedure return call
        service->addProcedureReturn("Hello", new ProcedureReturn(
                std::bind(&HelloServerStub::HelloStub, this, _1, _2),
                "user", json::TYPE_STRING
        ));

        // add notification
        service->addProcedureNotify("Goodbye", new ProcedureNotify(
                std::bind(&HelloServerStub::GoodbyeStub, &convert(), _1)
        ));
        server_.addService("Hello", service);
    }
    ~HelloServerStub() {}

public:
    void start() { server_.start(); }
    void setNumThread(size_t n) { server_.setNumThread(n); }

private:
    void HelloStub(json::Value &request, json::Value &response)
    {
        auto& params = request["params"];

        auto&& param0 = [&params]()->json::Value& {
            if (params.isArray())
                return params[0];
            return params["user"];
        }();

        auto result = convert().Hello(param0.getString());

        response.setObject();
        response.addMember("jsonrpc", "2.0");
        response.addMember("result", std::move(result));
        response.addMember("id", std::move(request["id"]));
    }

    void GoodbyeStub(json::Value &request)
    {
        convert().Goodbye(request);
    }

private:
    S& convert()
    {
        return static_cast<S&>(*this);
    }

private:
    RpcServer server_;
};

}


#define HelloServerImpl class HelloServer: public jrpc::HelloServerStub<HelloServer>

#endif //JRPC_SERVERSTUB_H
