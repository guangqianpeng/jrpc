//
// Created by frank on 18-1-25.
//

#include <random>
#include <iostream>

#include <example/arithmetic/ArithmeticClientStub.h>

using namespace jrpc;

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd());
std::uniform_int_distribution dis(0, 10);

void run(ArithmeticClientStub& client)
{
    double lhs = dis(gen);
    double rhs = dis(gen);

    client.Add(lhs, rhs, [=](json::Value response, bool isError, bool timeout) {
        if (!isError) {
            std::cout << lhs << "/" << rhs << "="
                      << response.getDouble() << "\n";
        }
        else if (timeout) {
            std::cout << "timeout\n";
        }
        else {
            std::cout << "response: "
                      << response["message"].getStringView() << ": "
                      << response["data"].getStringView() << "\n";
        }
    });
}

int main()
{
    EventLoop loop;
    InetAddress addr(9877);
    ArithmeticClientStub client(&loop, addr);

    client.setConnectionCallback([&](const TcpConnectionPtr& conn) {
        if (conn->disconnected()) {
            loop.quit();
        }
        else {
            loop.runEvery(500ms, [&] {
                run(client);
            });
        }
    });
    client.start();
    loop.loop();
}