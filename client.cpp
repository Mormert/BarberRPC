#include "common.h"

#include <iostream>
#include "rpc/client.h"

int main() {
    rpc::client client("127.0.0.1", 8080);

    auto enterBarbershop = client.call("enter_barbershop").as<EnterBarbershopInfo>();
    if (enterBarbershop.mBarbershopStatus == BarbershopStatus_FULL) {
        std::cout << "Barbershop had no empty waiting chairs! Exiting..." << std::endl;
    } else {
        Client me = enterBarbershop.mClient;

        auto serve = client.async_call("serve_me", me);

        std::future_status status;
        int i = 0;
        std::cout << "I am client number " << me << " and I'm being served... ";
        do {
            using namespace std::chrono_literals;
            status = serve.wait_for(1s);

            std::cout << ++i << ' ' << std::flush;
        } while (status != std::future_status::ready);

        std::cout << "\nFinished! Cost: " << serve.get().as<HaircutCost>() << " SEK." << std::endl;
    }

    std::cin.clear();
    std::cin.ignore(INT_MAX, '\n');
    std::cin.get();

    return 0;
}