//
// Created by frank on 18-4-11.
//

#include "example/n_queen/NQueenServiceStub.h"

using namespace jrpc;

class BackTrack
{
public:
    /*
     * 11 queen: 1ms
     * 12 queen: 7ms
     * 13 queen: 61ms
     * 14 queen: 240ms
     * 15 queen: 1534ms
     * 16 queen: 10441ms
     * */
    static int64_t solve(int nQueen)
    {
        if (nQueen < 0 || nQueen > BackTrack::kMaxQueens)
            return -1;

        BackTrack b(nQueen);
        b.search();
        return b.count;
    }

private:
    const static int kMaxQueens = 32;

    const int N;
    int64_t count;
    // bitmask, 0 is available
    uint32_t col[kMaxQueens];
    uint32_t diag[kMaxQueens];
    uint32_t antidiag[kMaxQueens];

    explicit
    BackTrack(int nQueens)
            : N(nQueens)
            , count(0)
            , col{0}
            , diag{0}
            , antidiag{0}
    {
        assert(nQueens <= kMaxQueens && nQueens > 0);
    }


    void search(int row = 0)
    {
        uint32_t avail = col[row] | diag[row] | antidiag[row];
        avail = ~avail; // bitmask, 1 is available

        while (avail) {

            int i = __builtin_ctz(avail);
            if (i >= N) break;

            if (row == N - 1)
                count++;
            else {
                uint32_t mask = (1u << i);
                col[row + 1] = (col[row] | mask);
                diag[row + 1] = ((diag[row] | mask) >> 1);
                antidiag[row + 1] = ((antidiag[row] | mask) << 1);
                search(row + 1);
            }
            avail &= avail - 1;
        }
    }
};


class NQueenService: public jrpc::NQueenServiceStub<NQueenService>
{
public:
    NQueenService(RpcServer& server, size_t workerThreadNum)
            : NQueenServiceStub(server)
            , pool_(workerThreadNum)
    {}

    void Solve(int nQueen, const UserDoneCallback& done)
    {
        pool_.runTask([=](){
            int64_t ans = BackTrack::solve(nQueen);
            done(json::Value(ans));
        });
    }

private:
    ThreadPool pool_;
};

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: ./n_queen_server #workerThread\n");
        exit(EXIT_FAILURE);
    }
    size_t wokerThreadNum = std::stoul(argv[1]);

//    for (int i = 8; i < 20; i++)
//    {
//        auto start = ev::clock::now();
//        BackTrack::solve(i);
//        auto delay = ev::clock::now() - start;
//        INFO("%d queen: %ldms", i, delay.count() / 1000000);
//    }

    EventLoop loop;
    InetAddress addr(9877);

    RpcServer rpcServer(&loop, addr);
    NQueenService service(rpcServer, wokerThreadNum);

    rpcServer.start();
    loop.loop();
}