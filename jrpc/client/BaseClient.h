//
// Created by frank on 17-12-31.
//

#ifndef JRPC_BASECLIENT_H
#define JRPC_BASECLIENT_H

#include <jackson/Value.h>

#include <jrpc/util.h>

namespace jrpc
{

class BaseClient: noncopyable
{
public:
    typedef std::function<void(json::Value&,
                               bool isError,
                               bool isTimeout)> ResponseCallback;

public:
    BaseClient(EventLoop* loop, const InetAddress& serverAddress):
            id_(0),
            client_(loop, serverAddress)
    {
        client_.setMessageCallback(std::bind(
                &BaseClient::onMessage, this, _1, _2));
    }

    void start() { client_.start(); }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        client_.setConnectionCallback(cb);
    }

    void sendRequest(const TcpConnectionPtr& conn, json::Value& request,
                     const ResponseCallback& cb);

    void sendNotification(const TcpConnectionPtr& conn, json::Value& request);

private:
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void handleMessage(Buffer& buffer);
    void handleResponse(std::string& json);
    void handleSingleResponse(json::Value& response);
    void validateResponse(json::Value& response);
    void sendJsonValue(const TcpConnectionPtr& conn, json::Value& value);

private:
    typedef std::unordered_map<int64_t, ResponseCallback> Callbacks;
    int64_t id_;
    Callbacks callbacks_;
    TcpClient client_;
};


}


#endif //JRPC_BASECLIENT_H
