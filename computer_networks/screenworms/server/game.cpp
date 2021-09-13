#include <cmath>
#include <algorithm>
#include "game.h"

namespace {
constexpr uint32_t RANDOM_MULTIPLIER = 279410273;
constexpr uint32_t RANDOM_MODULO = 4294967291;

constexpr double INITIAL_POSITION_OFFSET = 0.5;
constexpr int DIRECTION_MOD = 360;

constexpr double DEGREES_TO_RADIANS = 3.141592 / 180;

constexpr int FIRST_EVENT_ID = 0;

constexpr int END_GAME_PLAYERS_COUNT = 1;
}

namespace screen_worms {
uint32_t Game::get_random_value() {
    uint64_t old = random_var;
    uint64_t temp = random_var * RANDOM_MULTIPLIER;
    random_var = static_cast<uint32_t>(temp % RANDOM_MODULO);
    return static_cast<uint32_t>(old);
}

bool Game::is_player_out_of_bounds(double x, double y) const {
    return x >= max_x || x < 0 || y >= max_y || y < 0;
}

void Game::generate_new_game() {
    // It is always the first event in the game and it occurs only once per game.
    datagrams.emplace_back(game_id, FIRST_EVENT_ID);
    datagrams.back().add_new_game(players, max_x, max_y);
}

void Game::generate_pixel(uint32_t x, uint32_t y, int8_t player_number) {
    // If current datagram can't fit it, create a new one.
    if (!datagrams.back().add_pixel(player_number, x, y)) {
        datagrams.emplace_back(game_id, datagrams.back().get_bounds().second + 1);
        datagrams.back().add_pixel(player_number, x, y);
    }
    board[x][y] = true;
}

void Game::generate_player_eliminated(int8_t player_number) {
    // If current datagram can't fit it, create a new one.
    if (!datagrams.back().add_player_eliminated(player_number)) {
        datagrams.emplace_back(game_id, datagrams.back().get_bounds().second + 1);
        datagrams.back().add_player_eliminated(player_number);
    }

    active_players--;
    if (active_players == END_GAME_PLAYERS_COUNT) {
        generate_game_over();
    }
}

void Game::generate_game_over() {
    // If current datagram can't fit it, create a new one.
    if (!datagrams.back().add_game_over()) {
        datagrams.emplace_back(game_id, datagrams.back().get_bounds().second + 1);
        datagrams.back().add_game_over();
    }
    is_active = false;

    // Remove disconnected players.
    bool del = false;
    for (auto curr = players.begin(); curr != players.end(); curr = std::next(curr)) {
        if (del) {
            players.erase(std::prev(curr));
        }

        del = curr->second.is_disconnected;
    }

    if (del) {
        players.erase(std::prev(players.end()));
    }
}

void Game::create_new_game() {
    board = std::vector<std::vector<bool>>(max_x, std::vector<bool>(max_y, false));
    datagrams = std::vector<EventsDatagram>();
    game_id = get_random_value();
    is_active = true;
    active_players = static_cast<int>(players.size());
    int8_t player_no = 0;
    uint32_t x, y;

    generate_new_game();

    for (auto &[_, player] : players) {
        x = (get_random_value() % max_x);
        y = (get_random_value() % max_y);

        player.x = x + INITIAL_POSITION_OFFSET;
        player.y = y + INITIAL_POSITION_OFFSET;
        player.is_eliminated = false;
        player.direction = static_cast<int>(get_random_value() % DIRECTION_MOD);
        player.direction_rad = player.direction * DEGREES_TO_RADIANS;
        player.player_number = player_no;
        player_no++;

        if (board[x][y]) {
            generate_player_eliminated(player.player_number);
            player.is_eliminated = true;
        } else {
            generate_pixel(x, y, player.player_number);
        }
    }
}

bool Game::play_round() {
    // This function should not be called if game is inactive,
    // but it's the caller responsibility to check it.
    // Returns true if the game can continue after this round.
    uint32_t prev_x, prev_y, new_x, new_y;
    bool did_game_finish = true;

    for (auto &[_, player] : players) {
        if (!player.is_eliminated) {
            prev_x = static_cast<uint32_t>(player.x);
            prev_y = static_cast<uint32_t>(player.y);

            if (player.turning_direction == TURN_DIRECTION_RIGHT) {
                player.turning_direction =
                        (player.turning_direction + static_cast<int>(turning_speed))
                                % DIRECTION_MOD;
            } else if (player.turning_direction == TURN_DIRECTION_LEFT) {
                player.turning_direction =
                        (player.turning_direction - static_cast<int>(turning_speed))
                                % DIRECTION_MOD;
            }
            player.direction_rad = player.direction * DEGREES_TO_RADIANS;

            player.x += std::cos(player.direction_rad);
            player.y += std::sin(player.direction_rad);

            new_x = static_cast<uint32_t>(player.x);
            new_y = static_cast<uint32_t>(player.y);

            if (prev_x != new_x || prev_y != new_y) {
                if (is_player_out_of_bounds(player.x, player.y) || board[new_x][new_y]) {
                    generate_player_eliminated(player.player_number);
                    player.is_eliminated = true;
                    if (!is_active) {
                        // The game has ended.
                        did_game_finish = false;
                    }
                } else {
                    generate_pixel(new_x, new_y, player.player_number);
                }
            }
        }
    }

    return did_game_finish;
}

const std::vector<EventsDatagram> &Game::get_datagrams() const {
    return datagrams;
}

bool Game::is_game_active() const {
    return is_active;
}

void Game::add_new_player(const std::string &name) {
    // The server is responsible for checking if there is no such player.
    GamePlayer new_player{};

    auto player_it = players.insert({name, new_player});
    if (!player_it.second) {
        print_log("Adding a new player failed.");
    }
    player_it.first->second.is_disconnected = false;
    // If game is active, join as an eliminated player.
    player_it.first->second.is_eliminated = is_active;
}

void Game::change_player_turning_direction(const std::string &name, int dir) {
    players[name].turning_direction = dir;
}

void Game::delete_player(const std::string &name) {
    // We assume that player with such name exists, server should already have checked it.
    if (is_active) {
        players[name].is_disconnected = true;
    } else {
        players.erase(name);
    }
}
}