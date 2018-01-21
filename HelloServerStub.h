//
// Created by frank on 17-12-31.
//

#ifndef JRPC_SERVERSTUB_H
#define JRPC_SERVERSTUB_H


#include <jackson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcServer.h>
#include <jrpc/server/RpcService.h>

class HelloServer;

namespace jrpc
{

// 75\r\n{"jsonrpc": "2.0", "method": "Hello.Hello", "params": ["frank"], "id": 1}\r\n
// 77\r\n[{"jsonrpc": "2.0", "method": "Hello.Hello", "params": ["frank"], "id": 1}]\r\n
// 47\r\n{"jsonrpc": "2.0", "method": "Hello.Goodbye"}\r\n

typedef std::function<void(std::string&)> HelloDoneCallback;

template <typename S>
class HelloServerStub: noncopyable
{
protected:
    HelloServerStub(EventLoop* loop, const InetAddress listen):
            server_(loop, listen)
    {
        static_assert(std::is_same_v<S, HelloServer>,
                      "derived class name should be 'HelloServer'");

        auto service = new RpcService;

        // add procedure return call
        service->addProcedureReturn("Hello", new ProcedureReturn(
                std::bind(&HelloServerStub::HelloStub, this, _1, _2),
                "user", json::TYPE_STRING
        ));
        service->addProcedureReturn("Echo", new ProcedureReturn(
                std::bind(&HelloServerStub::EchoStub, this, _1, _2),
                "msg", json::TYPE_STRING
        ));
        // add notification
        service->addProcedureNotify("Goodbye", new ProcedureNotify(
                std::bind(&HelloServerStub::GoodbyeStub, this)
        ));
        // register service
        server_.addService("Hello", service);
    }
    ~HelloServerStub() = default;

public:
    void start() { server_.start(); }
    void setNumThread(size_t n) { server_.setNumThread(n); }

private:
    void HelloStub(json::Value request, const RpcDoneCallback& done)
    {
        auto &params = request["params"];

        auto &&param0 = [&params]() -> json::Value & {
            if (params.isArray())
                return params[0];
            return params["user"];
        }();

        auto result = convert().Hello(param0.getString());
        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("id", request["id"]);
        response.addMember("result", result);

        done(response);
    }

    void EchoStub(json::Value request, const RpcDoneCallback& done)
    {
        auto& params = request["params"];

        auto&& param0 = [&params]()->json::Value& {
            if (params.isArray())
                return params[0];
            return params["msg"];
        }();

        auto result = convert().Echo(param0.getString());

        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("result", result);
        response.addMember("id", request["id"]);

        done(response);
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

private:
    RpcServer server_;
};

}


#define HelloServerImpl HelloServer: public jrpc::HelloServerStub<HelloServer>

#endif //JRPC_SERVERSTUB_H
