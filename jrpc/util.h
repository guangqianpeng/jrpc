//
// Created by frank on 17-12-31.
//

#ifndef JRPC_UTIL_H
#define JRPC_UTIL_H

#include <string>
#include <string_view>

#include <jackson/Value.h>

#include <tinyev/EventLoop.h>
#include <tinyev/TcpConnection.h>
#include <tinyev/TcpServer.h>
#include <tinyev/TcpClient.h>
#include <tinyev/InetAddress.h>
#include <tinyev/Buffer.h>
#include <tinyev/Logger.h>
#include <tinyev/Callbacks.h>
#include <tinyev/Timestamp.h>
#include <tinyev/ThreadPool.h>
#include <tinyev/CountDownLatch.h>

namespace jrpc
{

using namespace std::literals::string_view_literals;
using namespace std::literals::chrono_literals;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

using ev::EventLoop;
using ev::TcpConnection;
using ev::TcpServer;
using ev::TcpClient;
using ev::InetAddress;
using ev::TcpConnectionPtr;
using ev::noncopyable;
using ev::Buffer;
using ev::ConnectionCallback;
using ev::ThreadPool;
using ev::CountDownLatch;

typedef std::function<void(json::Value response)> RpcDoneCallback;

class UserDoneCallback
{
public:
    UserDoneCallback(json::Value &request,
                     const RpcDoneCallback &callback)
            : request_(request),
              callback_(callback)
    {}


    void operator()(json::Value &&result) const
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

}

#endif //JRPC_UTIL_H
