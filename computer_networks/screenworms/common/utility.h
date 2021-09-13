#ifndef SIK2_COMMON_UTILITY_H_
#define SIK2_COMMON_UTILITY_H_

#include <iostream>

#define syserr(mess)                                                        \
        std::cerr << "ERROR " << errno << ": " << mess << std::endl;        \
        exit(EXIT_FAILURE)

#define print_log(mess) std::cout << "LOG: " << mess << std::endl

constexpr int NO_FLAGS = 0;

constexpr int TURN_DIRECTION_STRAIGHT = 0;
constexpr int TURN_DIRECTION_RIGHT = 1;
constexpr int TURN_DIRECTION_LEFT = 2;

constexpr int SERVER_MAX_DATAGRAM_SIZE = 550;

using session_id_t = uint64_t;
using game_id_t = uint32_t;

uint32_t calculate_crc32(const char* data, size_t count);

#endif //SIK2_COMMON_UTILITY_H_
