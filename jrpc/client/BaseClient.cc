//
// Created by frank on 17-12-31.
//

#include <jackson/Document.h>
#include <jackson/StringWriteStream.h>
#include <jackson/Writer.h>

#include <jrpc/client/BaseClient.h>
#include <jrpc/Exception.h>

using namespace jrpc;

namespace
{

const size_t kMaxMessageLen = 65536;

json::Value& findValue(json::Value &value, const char *key, json::ValueType type)
{
    auto it = value.findMember(key);
    if (it == value.memberEnd()) {
        throw ResponseException("missing at least one field");
    }
    if (it->value.getType() != type) {
        throw ResponseException("bad type of at least one field");
    }
    return it->value;
}

json::Value& findValue(json::Value& value, const char* key,
                       json::ValueType type, int32_t id)
{
    try {
        return findValue(value, key, type);
    }
    catch (ResponseException& e) {
        throw ResponseException(e.what(), id);
    }
}

}

void BaseClient::sendCall(const TcpConnectionPtr& conn, json::Value& call,
                          const ResponseCallback& cb)
{
    // remember callback when recv response
    call.addMember("id", id_);
    callbacks_[id_] = cb;
    id_++;

    sendRequest(conn, call);
}

void BaseClient::sendNotify(const TcpConnectionPtr& conn, json::Value& notify)
{
    sendRequest(conn, notify);
}

void BaseClient::sendRequest(const TcpConnectionPtr& conn, json::Value& request)
{
    json::StringWriteStream os;
    json::Writer writer(os);
    request.writeTo(writer);

    // wish sso string don't allocate heap memory...
    auto message = std::to_string(os.get().length() + 2)
            .append("\r\n")
            .append(os.get())
            .append("\r\n");
    conn->send(message);
}


void BaseClient::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    try {
        handleMessage(buffer);
    }
    catch (ResponseException& e) {
        ERROR("response error: %s", e.what());
        if (e.hasId()) {
            // fixme: should we?
            callbacks_.erase(e.Id());
        }
    }
}

void BaseClient::handleMessage(Buffer& buffer)
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
            // retrieve trash line for debugging
            buffer.retrieve(headerLen);
            throw ResponseException("invalid message length");
        }

        auto bodyLen = static_cast<uint32_t>(header.getInt32());
        if (bodyLen >= kMaxMessageLen)
            throw ResponseException("message is too long");

        if (buffer.readableBytes() < headerLen + bodyLen)
            break;
        buffer.retrieve(headerLen);
        auto json = buffer.retrieveAsString(bodyLen);
        handleResponse(json);
    }
}

void BaseClient::handleResponse(std::string& json)
{
    json::Document response;
    json::ParseError err = response.parse(json);
    if (err != json::PARSE_OK)
        throw ResponseException(json::parseErrorStr(err));

    switch (response.getType()) {
        case json::TYPE_OBJECT:
            handleSingleResponse(response);
            break;
        case json::TYPE_ARRAY: {
            size_t n = response.getSize();
            if (n == 0)
                throw ResponseException("batch response is empty");
            for (size_t i = 0; i < n; i++)
                handleSingleResponse(response[i]);

            break;
        }
        default:
            throw ResponseException("response should be json object or array");
    }
}


void BaseClient::handleSingleResponse(json::Value& response)
{
    validateResponse(response);
    auto id = response["id"].getInt32();

    auto it = callbacks_.find(id);
    if (it == callbacks_.end()) {
        WARN("response %d not found in stub", id);
        return;
    }

    auto result = response.findMember("result");
    if (result != response.memberEnd()) {
        it->second(result->value, false, false);
    }
    else {
        auto error = response.findMember("error");
        assert(error != response.memberEnd());
        it->second(error->value, true, false);
    }
    callbacks_.erase(it);
}

void BaseClient::validateResponse(json::Value& response)
{
    if (response.getSize() != 3) {
        throw ResponseException("response should have exactly 3 field"
                                        "(jsonrpc, error/result, id)");
    }

    auto id = findValue(response, "id",
                        json::TYPE_INT32).getInt32();

    auto version = findValue(response, "jsonrpc",
                             json::TYPE_STRING, id).getStringView();
    if (version != "2.0") {
        throw ResponseException("unknown json rpc version", id);
    }

    if (response.findMember("result") != response.memberEnd())
        return;

    findValue(response, "error", json::TYPE_OBJECT, id);
}

