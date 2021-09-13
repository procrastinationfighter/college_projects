#ifndef SIK2_SERVER_GAME_TYPES_H_
#define SIK2_SERVER_GAME_TYPES_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "../common/utility.h"

namespace screen_worms {
struct GamePlayer {
    double x;
    double y;
    bool is_eliminated;
    int direction;
    int8_t player_number;
    int turning_direction;
    double direction_rad;
    bool is_disconnected;
};

class EventsDatagram {
  private:
    std::vector<char> datagram;
    size_t curr_size;
    std::pair<uint32_t, uint32_t> bounds;  // The first and the last event id of events placed here.

    // Used for memcpy.
    uint32_t coordinates[2];

    void add_event_no();
  public:
    EventsDatagram(game_id_t game_id, uint32_t first_event_id);
    // These methods return true if adding was successful and false otherwise.
    bool add_new_game(const std::map<std::string, GamePlayer> &players,
                      uint32_t max_x,
                      uint32_t max_y);
    bool add_pixel(int8_t player_number, uint32_t x, uint32_t y);
    bool add_player_eliminated(int8_t player_number);
    bool add_game_over();

    const char *get_datagram() const;
    uint32_t get_datagram_size() const;
    const std::pair<uint32_t, uint32_t> &get_bounds() const;
};
}
#endif //SIK2_SERVER_GAME_TYPES_H_
