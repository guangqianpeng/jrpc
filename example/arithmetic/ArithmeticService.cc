//
// Created by frank on 18-1-25.
//

#include <example/arithmetic/ArithmeticServiceStub.h>

using namespace jrpc;

class ArithmeticService: public ArithmeticServiceStub<ArithmeticService>
{
public:
    explicit
    ArithmeticService(RpcServer& server):
            ArithmeticServiceStub(server),
            pool_(4)
    {}

    void Add(double lhs, double rhs, const UserDoneCallback& cb)
    {
        pool_.runTask([=](){
            cb(json::Value(lhs + rhs));
        });
    }

    void Sub(double lhs, double rhs, const UserDoneCallback& cb)
    {
        pool_.runTask([=](){
            cb(json::Value(lhs - rhs));
        });
    }

    void Mul(double lhs, double rhs, const UserDoneCallback& cb)
    {
        pool_.runTask([=](){
            cb(json::Value(lhs * rhs));
        });
    }

    void Div(double lhs, double rhs, const UserDoneCallback& cb)
    {
        pool_.runTask([=](){
            cb(json::Value(lhs / rhs));
        });
    }

private:
    ThreadPool pool_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(9877);

    RpcServer rpcServer(&loop, addr);
    ArithmeticService service(rpcServer);

    rpcServer.start();
    loop.loop();
}