//
// Created by frank on 17-12-29.
//


#include <jackson/Document.h>
#include <jackson/Writer.h>
#include <jackson/StringWriteStream.h>


#include <jrpc/BufferWriteStream.h>
#include <jrpc/Exception.h>
#include <jrpc/server/BaseServer.h>
#include <jrpc/server/RpcServer.h>

using namespace jrpc;

namespace
{

const size_t kHighWatermark = 65536;
const size_t kMaxMessageLen = 1024;

}

namespace jrpc
{

template class BaseServer<RpcServer>;

}

template <typename ProtocolServer>
BaseServer<ProtocolServer>::BaseServer(EventLoop *loop, const InetAddress &listen)
        :server_(loop, listen)
{
    server_.setConnectionCallback(std::bind(
            &BaseServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(
            &BaseServer::onMessage, this, _1, _2));
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        DEBUG("connection %s is [up]", 
              conn->peer().toIpPort().c_str());
        conn->setHighWaterMarkCallback(std::bind(
                &BaseServer::onHighWatermark, this, _1, _2), kHighWatermark);
    }
    else {
        DEBUG("connection %s is [down]",
              conn->peer().toIpPort().c_str());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    try {
        handleMessage(conn, buffer);
    }
    catch (RequestException& e) {
        json::Value response;
        wrapException(response, e);
        sendResponse(conn, response);
        conn->shutdown();

        WARN("BaseServer::onMessage() %s request error: %s",
             conn->peer().toIpPort().c_str(), e.what());
    }
    catch (NotifyException& e)
    {
        WARN("BaseServer::onMessage() %s notify error: %s",
             conn->peer().toIpPort().c_str(), e.what());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onHighWatermark(const TcpConnectionPtr& conn, size_t mark)
{
    WARN("connection %s high watermark %lu",
         conn->peer().toIpPort().c_str(), mark);

    conn->setWriteCompleteCallback(std::bind(
            &BaseServer::onWriteComplete, this, _1));
    conn->stopRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onWriteComplete(const TcpConnectionPtr& conn)
{
    INFO("connection %s write complete",
         conn->peer().toIpPort().c_str());
    conn->startRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::handleMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    while (true) {

        const char *crlf = buffer.findCRLF();
        if (crlf == nullptr)
            break;

        size_t headerLen = crlf - buffer.peek() + 2;

        json::Document header;
        auto err = header.parse(buffer.peek(), headerLen);
        if (err != json::PARSE_OK ||
            !header.isInt32() ||
            header.getInt32() <= 0)
        {
            throw RequestException(RPC_INVALID_REQUEST, "invalid message length");
        }

        auto bodyLen = static_cast<uint32_t>(header.getInt32());
        if (bodyLen >= kMaxMessageLen)
            throw RequestException(RPC_INVALID_REQUEST, "message is too long");

        if (buffer.readableBytes() < headerLen + bodyLen)
            break;

        json::Value response;
        buffer.retrieve(headerLen);
        auto json = buffer.retrieveAsString(bodyLen);
        convert().handleRequest(json, response);
        if (!response.isNull()) {
            sendResponse(conn, response);
            DEBUG("BaseServer::handleMessage() %s request success,",
                 conn->peer().toIpPort().c_str());
        }
        else {
            DEBUG("BaseServer::handleMessage() %s notify success,",
                 conn->peer().toIpPort().c_str());
        }
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::wrapException(json::Value& response, RequestException& e)
{
    assert(response.isNull());

    response.setObject();
    response.addMember("jsonrpc", "2.0");
    auto& value = response.addMember("error", json::TYPE_OBJECT);
    value.addMember("code", e.err().asCode());
    value.addMember("message", e.err().asString());
    value.addMember("data", e.detail());
    response.addMember("id", std::move(e.id()));

}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::sendResponse(const TcpConnectionPtr& conn, json::Value& response)
{
    json::StringWriteStream os;
    json::Writer writer(os);
    response.writeTo(writer);

    // wish sso string don't allocate heap memory...
    conn->send(std::to_string(os.get().length() + 2));
    conn->send("\r\n"sv);
    conn->send(os.get());
    conn->send("\r\n"sv);
}

template <typename ProtocolServer>
ProtocolServer& BaseServer<ProtocolServer>::convert()
{
    return static_cast<ProtocolServer&>(*this);
}

template <typename ProtocolServer>
const ProtocolServer& BaseServer<ProtocolServer>::convert() const
{
    return static_cast<const ProtocolServer&>(*this);
}


