//
// Created by frank on 17-12-31.
//

#include "HelloServiceStub.h"

using namespace jrpc;

class HelloService: public jrpc::HelloServiceStub<HelloService>
{
public:
    explicit
    HelloService(RpcServer& server):
            HelloServiceStub(server),
            pool_(1)
    {}

    void Hello(std::string user, const HelloDoneCallback& done)
    {
        pool_.runTask([=](){
            auto result = std::string("hello, ").append(user);
            done(std::move(result));
        });
    }

    void Add(int32_t lhs, int32_t rhs, const AddDoneCallback& done)
    {
        done(lhs + rhs);
    }

    void Goodbye()
    {
        INFO("good bye");
    }

private:
    ThreadPool pool_;
};

int main()
{
    EventLoop loop;
    InetAddress listen(9877);
    RpcServer rpcServer(&loop, listen);

    HelloService server(rpcServer);
    //... other service can be added here

    rpcServer.start();
    loop.loop();
}