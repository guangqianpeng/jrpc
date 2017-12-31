//
// Created by frank on 17-12-31.
//

#include <iostream>

#include "HelloClientStub.h"

using namespace jrpc;

void run(HelloClient& client)
{
    client.Echo("蛤蛤蛤", [](json::Value &response, bool isError, bool timeout) {
        if (isError) {
            std::cout << "response: " << response["message"].getString();
        } else {
            std::cout << "response: " << response.getString() << "\n";
        }
    });
}

int main()
{
    EventLoop loop;
    InetAddress serverAddr(9877);
    HelloClient client(&loop, serverAddr);

    client.setConnectionCallback([&](const TcpConnectionPtr& conn) {
        if (conn->disconnected()) {
            loop.quit();
        }
        loop.runEvery(1s, [&] {
            run(client);
        });
    });

    client.start();
    loop.loop();
}