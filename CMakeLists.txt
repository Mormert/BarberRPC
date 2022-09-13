cmake_minimum_required(VERSION 3.20)
project(Barber)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty/rpclib)
include_directories(3rdparty/rpclib/include
                    3rdparty/concurrentqueue)

add_executable(BarberServer server.cpp)
target_link_libraries(BarberServer PRIVATE rpc)

add_executable(BarberClient client.cpp)
target_link_libraries(BarberClient PRIVATE rpc)
