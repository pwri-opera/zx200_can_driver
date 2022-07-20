cmake_minimum_required(VERSION 3.8)
project(zx200_can_driver VERSION 2 LANGUAGES CXX)

find_package(canary REQUIRED)

add_executable(c3msg_can_relay c3msg_can_relay.cpp)
target_link_libraries(c3msg_can_relay PUBLIC canary::canary)
