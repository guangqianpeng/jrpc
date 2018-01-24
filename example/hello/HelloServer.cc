//
// Created by frank on 17-12-31.
//

#include "example/hello/HelloServiceStub.h"

using namespace jrpc;

class HelloService: public jrpc::HelloServiceStub<HelloService>
{
public:
    explicit
    HelloService(RpcServer& server):
            HelloServiceStub(server),
            pool_(2)
    {}

    void Hello(std::string user, const UserDoneCallback& done)
    {
        std::string result;
        if (user == "苟利国家生死以") {
            result = "岂因祸福避趋之?";
        } else {
            result = std::string("hello, ").append(user);
        }
        done(json::Value(result));
    }

    void Add(int32_t lhs, int32_t rhs, const UserDoneCallback& done)
    {
        pool_.runTask([=](){
            done(json::Value(lhs + rhs));
        });
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