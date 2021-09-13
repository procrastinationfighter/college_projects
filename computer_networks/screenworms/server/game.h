#ifndef SIK2_SERVER_GAME_H_
#define SIK2_SERVER_GAME_H_

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include "game_types.h"
#include "../common/utility.h"

namespace screen_worms {
class Game {
  private:
    uint32_t random_var;
    const uint32_t turning_speed;
    const uint32_t max_x;
    const uint32_t max_y;

    game_id_t game_id;
    bool is_active;
    int active_players;
    std::vector<std::vector<bool>> board;
    std::map<std::string, GamePlayer> players;
    std::vector<EventsDatagram> datagrams;

    uint32_t get_random_value();
    bool is_player_out_of_bounds(double x, double y) const;

    void generate_new_game();
    void generate_pixel(uint32_t x, uint32_t y, int8_t player_number);
    void generate_player_eliminated(int8_t player_number);
    void generate_game_over();
  public:
    Game(uint32_t seed, uint32_t turning_speed, uint32_t width, uint32_t height)
            : random_var(seed),
              turning_speed(turning_speed),
              max_x(width),
              max_y(height),
              game_id(0),
              is_active(false) {}

    void create_new_game();

    // Returns number of the first event to send.
    bool play_round();
    const std::vector<EventsDatagram> &get_datagrams() const;
    bool is_game_active() const;

    void add_new_player(const std::string &name);
    void change_player_turning_direction(const std::string &name, int dir);
    void delete_player(const std::string &name);
};
}
#endif //SIK2_SERVER_GAME_H_
