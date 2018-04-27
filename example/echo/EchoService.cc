//
// Created by frank on 18-1-24.
//

#include "example/echo/EchoServiceStub.h"

using namespace jrpc;

class EchoService: public jrpc::EchoServiceStub<EchoService>
{
public:
    explicit
    EchoService(RpcServer& server):
            EchoServiceStub(server)
    {}

    void Echo(std::string message, const UserDoneCallback& done)
    {
        done(json::Value(message));
    }
};

int main()
{
    EventLoop loop;
    InetAddress listen(9877);
    RpcServer rpcServer(&loop, listen);

    EchoService server(rpcServer);

    rpcServer.start();
    loop.loop();
}
