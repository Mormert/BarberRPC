#ifndef BARBER_COMMON_H
#define BARBER_COMMON_H

#include <type_traits>
#include <thread>
#include <iostream>
#include <array>
#include <semaphore>
#include <random>
#include <queue>
#include "blockingconcurrentqueue.h"
#include "rpc/server.h"

using namespace std::chrono_literals;

typedef uint8_t BarbershopStatus;
#define BarbershopStatus_FULL 0
#define BarbershopStatus_AVAILABLE 1

typedef int32_t Client;
typedef int32_t HaircutCost;

struct EnterBarbershopInfo {
    Client mClient;
    BarbershopStatus mBarbershopStatus;

    MSGPACK_DEFINE_ARRAY(mClient, mBarbershopStatus)
};

class Barber {
public:

    void SetupRPCs(rpc::server &server) {
        server.bind("enter_barbershop", [&]() -> EnterBarbershopInfo {
            std::lock_guard<std::mutex> lockGuard{mClientQueueStlMtx};
            if (mClientQueue.size() > mWaitingChairs) {
                return EnterBarbershopInfo{-1, BarbershopStatus_FULL};
            } else {
                return EnterBarbershopInfo{static_cast<Client>(mClientIncrementor++), BarbershopStatus_AVAILABLE};
            }
        });

        server.bind("serve_me", [&](Client client) -> HaircutCost {
            mClientQueueStlMtx.lock();
            mClientQueue.push(client);
            mClientQueueStlMtx.unlock();

            // Wait until the client has been served
            sBinarySemaphores.at(client % 3).acquire();

            std::random_device dev;
            std::mt19937 rng(dev());
            std::uniform_int_distribution<std::mt19937::result_type> dist(8, 50);
            return dist(rng) * 10;
        });
    }

    [[noreturn]] void ServeOrSleep() {
        while (true) {
            // Waits forever until there is some client waiting to be served (aka "sleep")


            mClientQueueStlMtx.lock();
            if(mClientQueue.empty())
            {
                mClientQueueStlMtx.unlock();
                continue;
            }
            Client c = mClientQueue.front();
            mClientQueue.pop();
            mClientQueueStlMtx.unlock();
            std::cout << "Serving client " << c << ": ";
            for (int i = 0; i < mClientServeTimeSeconds; i++) {
                std::this_thread::sleep_for(1s);
                std::cout << i << ' ';
            }
            std::cout << "... finished!" << std::endl;
            sBinarySemaphores.at(c % 3).release();
        }
    }


private:
    std::atomic<Client> mClientIncrementor{0};

    const static inline int mWaitingChairs{3};
    const static inline int mClientServeTimeSeconds{15};

    std::queue<Client> mClientQueue;
    std::mutex mClientQueueStlMtx;

    // This array of binary semaphores is used to signal the "serve_me" from the
    // ServeOrSleep function to continue the execution in "serve_me"
    std::array<std::binary_semaphore, 3> sBinarySemaphores = {std::binary_semaphore{0},
                                                              std::binary_semaphore{0},
                                                              std::binary_semaphore{0}};

};

#endif //BARBER_COMMON_H
