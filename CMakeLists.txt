cmake_minimum_required(VERSION 3.8)
project(zx200_can_driver VERSION 2 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)

# include_directories(
#     ${CMAKE_SOURCE_DIR}/src
#     ${CMAKE_BINARY_DIR}/src
# )


# file(GLOB header
#     "*.h"
# )
# file(GLOB src
#     "*.cpp"
# )


find_package(canary REQUIRED)
# find_package(Curses REQUIRED)

find_package(Boost 1.70
             COMPONENTS
                system
                timer
             REQUIRED)

find_package (Threads)

find_package(RTTR CONFIG REQUIRED Core)

# add_executable(zx200_can_keynav zx200_can_keynav.cpp)
# target_link_libraries(zx200_can_keynav PUBLIC canary::canary curses Boost::system
#         Threads::Threads boost_thread boost_coroutine)

add_executable(zx200_keynav zx200_keynav.cpp)
# set_target_properties(zx200_keynav PROPERTIES LINKER_LANGUAGE CXX)
# set_property(TARGET zx200_keynav PROPERTY CXX_STANDARD 17)
target_link_libraries(zx200_keynav PUBLIC canary::canary curses Boost::system
        Threads::Threads boost_thread boost_coroutine boost_timer dbcppp RTTR::Core)




# add_executable(c3msg_can_relay c3msg_can_relay.cpp)
# target_link_libraries(c3msg_can_relay PUBLIC canary::canary)
