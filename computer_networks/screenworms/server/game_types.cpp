#include <netinet/in.h>
#include <cstring>
#include "game_types.h"

namespace {
constexpr int8_t NEW_GAME_EVENT_NUMBER = 0;
constexpr int8_t PIXEL_EVENT_NUMBER = 1;
constexpr int8_t PLAYER_ELIMINATED_EVENT_NUMBER = 2;
constexpr int8_t GAME_OVER_EVENT_NUMBER = 3;

// We use the fact that there will be at most one new_game event.
constexpr uint32_t NEW_GAME_EVENT_SIZE_WITHOUT_PLAYERS_AND_CRC = 17;
constexpr uint32_t PIXEL_EVENT_LEN = 14;
constexpr uint32_t PLAYER_ELIMINATED_EVENT_LEN = 6;
constexpr uint32_t GAME_OVER_EVENT_LEN = 5;

constexpr size_t PIXEL_EVENT_SIZE = PIXEL_EVENT_LEN + 8;
constexpr size_t PLAYER_ELIMINATED_EVENT_SIZE = PLAYER_ELIMINATED_EVENT_LEN + 8;
constexpr size_t GAME_OVER_EVENT_SIZE = GAME_OVER_EVENT_LEN + 8;
}

namespace screen_worms {
EventsDatagram::EventsDatagram(game_id_t game_id, uint32_t first_event_id)
        : datagram(SERVER_MAX_DATAGRAM_SIZE),
          curr_size(sizeof(game_id_t)),
          bounds(first_event_id, first_event_id) {
    game_id = htonl(game_id);
    memcpy(datagram.data(), &game_id, sizeof(game_id_t));
}

// These functions base on the facts:
// First four bytes of an event are the event length.
// The next four bytes (5-8) are the event number.
// The next byte (9) is the event type.
// The next bytes are event type specific.
// The last four bytes are a control sum.

// Adds number of the next event to the datagram.
void EventsDatagram::add_event_no() {
    if (curr_size > sizeof(game_id_t)) {
        // If this event is not the first added to the datagram,
        // increase the second bound.
        // Otherwise, we can't do so, because the bounds are already in form {x, x},
        // where x is the number of the first event.
        bounds.second++;
    }
    uint32_t event_no = htonl(bounds.second);
    memcpy(datagram.data() + curr_size + 4, &event_no, sizeof(uint32_t));
}

bool EventsDatagram::add_new_game(const std::map<std::string, GamePlayer> &players,
                                  uint32_t max_x,
                                  uint32_t max_y) {
    // This will be always the first event, so we don't need to check if it will fit.
    add_event_no();

    datagram[curr_size + 8] = NEW_GAME_EVENT_NUMBER;

    coordinates[0] = htonl(max_x);
    coordinates[1] = htonl(max_y);
    memcpy(datagram.data() + curr_size + 9, coordinates, 2 * sizeof(uint32_t));

    uint32_t curr_len = NEW_GAME_EVENT_SIZE_WITHOUT_PLAYERS_AND_CRC;

    // Add every player's name.
    for (auto &[player_name, _] : players) {
        memcpy(datagram.data() + curr_size + curr_len,
               player_name.data(),
               player_name.size() * sizeof(char));
        curr_len += player_name.size();
        datagram[curr_size + curr_len] = '\0';
        curr_len++;
    }

    // Add length, it is equal to curr_len minus the first four bytes (those are the length).
    uint32_t len = htonl(curr_len - 4);
    memcpy(datagram.data() + curr_size, &len, sizeof(int32_t));

    uint32_t crc32 = htonl(calculate_crc32(datagram.data() + curr_size, curr_len));
    memcpy(datagram.data() + curr_len, &crc32, sizeof(uint32_t));

    // Set new curr_size, it will be curr_len plus four bytes of crc32.
    curr_size += curr_len + 4;

    return true;
}

bool EventsDatagram::add_pixel(int8_t player_number, uint32_t x, uint32_t y) {
    if (curr_size + PIXEL_EVENT_SIZE > SERVER_MAX_DATAGRAM_SIZE) {
        return false;
    }

    static uint32_t len = htonl(PIXEL_EVENT_LEN);
    memcpy(datagram.data() + curr_size, &len, sizeof(uint32_t));

    add_event_no();

    datagram[curr_size + 8] = PIXEL_EVENT_NUMBER;
    datagram[curr_size + 9] = player_number;

    coordinates[0] = htonl(x);
    coordinates[1] = htonl(y);
    memcpy(datagram.data() + curr_size + 10, coordinates, 2 * sizeof(uint32_t));

    uint32_t crc32 = htonl(calculate_crc32(datagram.data() + curr_size, PIXEL_EVENT_SIZE - 4));
    memcpy(datagram.data() + curr_size + 18, &crc32, sizeof(uint32_t));

    curr_size += PIXEL_EVENT_SIZE;

    return true;
}

bool EventsDatagram::add_player_eliminated(int8_t player_number) {
    if (curr_size + PLAYER_ELIMINATED_EVENT_SIZE > SERVER_MAX_DATAGRAM_SIZE) {
        return false;
    }

    static uint32_t len = htonl(PLAYER_ELIMINATED_EVENT_LEN);
    memcpy(datagram.data() + curr_size, &len, sizeof(uint32_t));

    add_event_no();

    datagram[curr_size + 8] = PLAYER_ELIMINATED_EVENT_NUMBER;
    datagram[curr_size + 9] = player_number;

    uint32_t crc32 =
            htonl(calculate_crc32(datagram.data() + curr_size, PLAYER_ELIMINATED_EVENT_SIZE - 4));
    memcpy(datagram.data() + curr_size + 10, &crc32, sizeof(uint32_t));

    curr_size += PLAYER_ELIMINATED_EVENT_SIZE;

    return true;
}

bool EventsDatagram::add_game_over() {
    if (curr_size + GAME_OVER_EVENT_SIZE > SERVER_MAX_DATAGRAM_SIZE) {
        return false;
    }

    static uint32_t len = htonl(GAME_OVER_EVENT_LEN);
    memcpy(datagram.data() + curr_size, &len, sizeof(uint32_t));

    add_event_no();

    datagram[curr_size + 8] = GAME_OVER_EVENT_NUMBER;

    uint32_t crc32 = htonl(calculate_crc32(datagram.data(), GAME_OVER_EVENT_SIZE - 4));
    memcpy(datagram.data() + curr_size + 9, &crc32, sizeof(uint32_t));

    curr_size += GAME_OVER_EVENT_SIZE;

    return true;
}

const char *EventsDatagram::get_datagram() const {
    return datagram.data();
}

uint32_t EventsDatagram::get_datagram_size() const {
    return curr_size;
}

const std::pair<uint32_t, uint32_t> &EventsDatagram::get_bounds() const {
    return bounds;
}
}