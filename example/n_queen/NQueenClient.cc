//
// Created by frank on 18-4-11.
//

#include <iostream>


#include "example/n_queen/NQueenClientStub.h"

using namespace jrpc;

void run(NQueenClientStub& client)
{
    static int n = 8;

    auto start = ev::clock::now();
    client.Solve(n, [&, start](json::Value response, bool isError, bool timeout) {
        if (timeout) {
            std::cout << "timeout\n";
        }
        else if (isError) {
            std::cout << "response: "
                      << response["message"].getStringView() << ": "
                      << response["data"].getStringView() << "\n";
        }
        auto delay = ev::clock::now() - start;
        INFO("queen = %d, response = %ld, delay = %ldms",
             n, response.getInt64(),
             delay.count() / 1000000);
        n++;
        run(client);
    });
}
/*
 *
20180411 07:14:18.061300 [23741] [INFO]  queen = 8, response = 92, delay = 0ms - NQueenClient.cc:23
20180411 07:14:18.061445 [23741] [INFO]  queen = 9, response = 352, delay = 0ms - NQueenClient.cc:23
20180411 07:14:18.061825 [23741] [INFO]  queen = 10, response = 724, delay = 0ms - NQueenClient.cc:23
20180411 07:14:18.063388 [23741] [INFO]  queen = 11, response = 2680, delay = 1ms - NQueenClient.cc:23
20180411 07:14:18.074156 [23741] [INFO]  queen = 12, response = 14200, delay = 10ms - NQueenClient.cc:23
20180411 07:14:18.113495 [23741] [INFO]  queen = 13, response = 73712, delay = 39ms - NQueenClient.cc:23
20180411 07:14:18.349997 [23741] [INFO]  queen = 14, response = 365596, delay = 236ms - NQueenClient.cc:23
20180411 07:14:19.889318 [23741] [INFO]  queen = 15, response = 2279184, delay = 1539ms - NQueenClient.cc:23
20180411 07:14:30.207867 [23741] [INFO]  queen = 16, response = 14772512, delay = 10318ms - NQueenClient.cc:23
 * 
 * */


int main()
{
    EventLoop loop;
    InetAddress serverAddr(9877);
    NQueenClientStub client(&loop, serverAddr);

    client.setConnectionCallback([&](const TcpConnectionPtr& conn) {
        if (conn->disconnected()) {
            loop.quit();
        }
        else {
            run(client);
        }
    });

    client.start();
    loop.loop();
}