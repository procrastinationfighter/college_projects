cmake_minimum_required(VERSION 3.19)
project(SiK2)

set(CMAKE_CXX_STANDARD 17)

add_executable(screen-worms-server
        server/server_main.cpp
        server/game_server.h
        server/game_server.cpp
        common/utility.h
        common/timer_poll.cpp
        common/timer_poll.h
        server/game.cpp
        server/game.h
        server/game_types.cpp
        server/game_types.h
        common/utility.cpp)

add_executable(screen-worms-client
        client/client_main.cpp
        common/utility.h
        common/timer_poll.cpp
        common/timer_poll.h
        common/utility.cpp)

# For debug purposes only
add_executable(test common/main.cpp common/timer_poll.h common/timer_poll.cpp common/utility.h common/utility.cpp)