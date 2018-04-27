//
// Created by frank on 18-1-24.
//

#include <iostream>

#include "example/any/AnyClientStub.h"

using namespace jrpc;

void run(AnyClientStub& client)
{
    static int counter = 0;
    counter++;

    std::string str = "苟利国家生死以+" + std::to_string(counter) + "s";

    json::Value object_data(json::TYPE_OBJECT);
    object_data.addMember("message", str);

    json::Value array_data(json::TYPE_ARRAY);

    client.Any(object_data, array_data, [](json::Value response, bool isError, bool timeout) {
        if (!isError) {
            std::cout << "response: " << response["message"].getStringView() << "\n";
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
    InetAddress serverAddr(9877);
    AnyClientStub client(&loop, serverAddr);

    client.setConnectionCallback([&](const TcpConnectionPtr& conn) {
        if (conn->disconnected()) {
            loop.quit();
        }
        else {
            loop.runEvery(1000ms, [&] {
                run(client);
            });
        }
    });

    client.start();
    loop.loop();
}