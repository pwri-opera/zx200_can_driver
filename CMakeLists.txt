cmake_minimum_required(VERSION 3.8)
project(zx200_can_driver VERSION 2 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 20)

find_package(canary REQUIRED)

find_package(Boost 1.70 REQUIRED)

find_package(RTTR CONFIG REQUIRED Core)

add_executable(zx200_keynav zx200_keynav.cpp)
set_property(TARGET zx200_keynav PROPERTY CXX_STANDARD 20)
target_link_libraries(zx200_keynav PUBLIC canary::canary
                        curses 
                        boost_system
                        boost_thread
                        boost_timer
                        dbcppp
                        RTTR::Core)