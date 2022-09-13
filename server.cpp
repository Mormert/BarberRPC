#include "common.h"

#include <iostream>
#include "rpc/server.h"

int main(int argc, char *argv[]) {
    const uint16_t port = 8080;
    rpc::server srv(port);

    Barber barber;
    barber.SetupRPCs(srv);

    // Run the server loop, using the maximum amount of threads the hardware supports.
    auto threads = std::thread::hardware_concurrency() - 1;
    if (threads <= 0) { threads = 1; }
    srv.async_run(threads);
    std::cout << "Running the server on port " << port << " with " << threads << " worker threads." << std::endl;

    barber.ServeOrSleep();
}