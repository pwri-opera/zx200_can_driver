cmake_minimum_required(VERSION 3.8)
project(zx200_can_driver VERSION 2 LANGUAGES CXX)

find_package(canary REQUIRED)
find_package(Boost 1.70
             COMPONENTS
                system
             REQUIRED)

find_package (Threads)

add_executable(zx200_can_keynav zx200_can_keynav.cpp)
target_link_libraries(zx200_can_keynav PUBLIC canary::canary curses Boost::system
        Threads::Threads boost_thread boost_coroutine)


add_executable(c3msg_can_relay c3msg_can_relay.cpp)
target_link_libraries(c3msg_can_relay PUBLIC canary::canary)
