//
// Created by frank on 17-12-31.
//

#ifndef JRPC_CLIENTSTUB_H
#define JRPC_CLIENTSTUB_H

#include <jackson/Value.h>

#include <jrpc/util.h>
#include <jrpc/client/BaseClient.h>

namespace jrpc
{

class HelloClientStub: noncopyable
{
public:
    HelloClientStub(EventLoop* loop, const InetAddress& serverAddress):
            client_(loop, serverAddress)
    {
        client_.setConnectionCallback([this](const TcpConnectionPtr& conn){
            if (conn->connected()) {
                INFO("connected");
                conn_ = conn;
                cb_(conn_);
            }
            else {
                INFO("disconnected");
                assert(conn_ != nullptr);
                cb_(conn_);
                conn_.reset();
            }
        });
    }

    void start() { client_.start(); }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        cb_ = cb;
    }
    
    void Hello(std::string_view user,
               const BaseClient::ResponseCallback& cb)
    {
        json::Value params(json::TYPE_OBJECT);
        params.addMember("user", user);
        HelloStub(params, cb);
    }

    void Add(int32_t lhs, int32_t rhs,
             const BaseClient::ResponseCallback &cb)
    {
        json::Value params(json::TYPE_OBJECT);
        params.addMember("lhs", lhs);
        params.addMember("rhs", rhs);
        AddStub(params, cb);
    }

    // notify
    void Goodbye()
    {
        GoodbyeStub();
    }

private:
    void HelloStub(json::Value& params,
                   const BaseClient::ResponseCallback& cb)
    {
        json::Value request(json::TYPE_OBJECT);
        request.addMember("jsonrpc", "2.0");
        request.addMember("method", "Hello.Hello");
        request.addMember("params", params);

        assert(conn_ != nullptr);
        client_.sendRequest(conn_, request, cb);
    }

    void AddStub(json::Value& params,
                 const BaseClient::ResponseCallback &cb)
    {
        json::Value request(json::TYPE_OBJECT);
        request.addMember("jsonrpc", "2.0");
        request.addMember("method", "Hello.Add");
        request.addMember("params", params);

        assert(conn_ != nullptr);
        client_.sendRequest(conn_, request, cb);
    };

    void GoodbyeStub()
    {
        json::Value request(json::TYPE_OBJECT);
        request.addMember("jsonrpc", "2.0");
        request.addMember("method", "Hello.Goodbye");

        assert(conn_ != nullptr);
        client_.sendNotification(conn_, request);
    }

private:
    TcpConnectionPtr conn_;
    ConnectionCallback cb_;
    BaseClient client_;
};

}


#endif //JRPC_CLIENTSTUB_H
