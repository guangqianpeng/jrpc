//
// Created by frank on 17-12-31.
//

#include "HelloServerStub.h"

using namespace jrpc;

HelloServerImpl
{
public:
    HelloServer(EventLoop* loop, const InetAddress& listen):
            HelloServerStub(loop, listen)
    {}

    std::string Hello(std::string_view user)
    {
        return std::string("hello, ").append(user);
    }

    std::string Echo(std::string_view msg)
    {
        return std::string(msg);
    }

    void Goodbye()
    {
        INFO("good bye");
    }

};

int main()
{
    EventLoop loop;
    InetAddress listen(9877);
    HelloServer server(&loop, listen);
    server.start();
    loop.loop();
}