//
// Created by frank on 18-1-24.
//

#include "example/any/AnyServiceStub.h"

using namespace jrpc;

class AnyService: public jrpc::AnyServiceStub<AnyService>
{
public:
    explicit
    AnyService(RpcServer& server):
            AnyServiceStub(server)
    {}

    void Any(json::Value object_data, json::Value array_data, const UserDoneCallback& done)
    {
        assert(object_data.getType() == json::TYPE_OBJECT);
        assert(array_data.getType() == json::TYPE_ARRAY);
        done(std::move(object_data));
    }
};

int main()
{
    EventLoop loop;
    InetAddress listen(9877);
    RpcServer rpcServer(&loop, listen);

    AnyService server(rpcServer);

    rpcServer.start();
    loop.loop();
}
