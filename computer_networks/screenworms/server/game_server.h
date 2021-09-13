#ifndef SIK2_SERVER_GAME_SERVER_H_
#define SIK2_SERVER_GAME_SERVER_H_

#include <cstdint>
#include <chrono>
#include <sys/socket.h>
#include <unordered_set>
#include <unordered_map>
#include <netinet/in.h>
#include "../common/timer_poll.h"
#include "game.h"
#include "../common/utility.h"

namespace screen_worms {
class Client {
  private:
    const session_id_t id;
    const std::string name;   // If empty, the client is a spectator.
    bool is_ready;
    std::chrono::time_point<std::chrono::system_clock> last_contact;
    struct sockaddr_storage address;
    uint32_t next_event_expected;
  public:
    Client(session_id_t session_id, std::string nickname, const sockaddr_storage *addr);
    // The elegant solution should friend the GameServer class and let it do most things,
    // but I have no time to repair it.
    bool is_spectator() const;
    void set_ready();
    void reset_ready();
    void update_contact();
    bool should_get_timeout(const std::chrono::time_point<std::chrono::system_clock> &now) const;
    bool are_names_equal(const std::string &name) const;
    bool is_session_id_greater(session_id_t id) const;
    bool is_session_id_equal(session_id_t id) const;
    const std::string &get_name() const;
    bool is_ready_to_play() const;
    void set_next_event_expected(uint32_t id);
    uint32_t get_next_event_expected() const;
    struct sockaddr_storage *get_address_ptr();
    socklen_t get_address_size();
};

using ClientMap = std::unordered_map<std::string, Client>;

class GameServer {
  private:
    TimerPoll timer_poll;
    Game game;
    std::unordered_set<std::string> taken_names;
    ClientMap clients;
    ClientMap::iterator next_datagram_target;

    static constexpr int CLIENT_DATAGRAM_MIN_LEN = 13;
    static constexpr int CLIENT_DATAGRAM_MAX_LEN = 33;
    uint8_t datagram_buffer[CLIENT_DATAGRAM_MAX_LEN];

    int not_ready_players;

    // If the game is not active, checks if the client wants to start the game
    // and if so, sets him as ready
    void set_client_ready_if_possible(Client &client, uint32_t turn_direction);

    void do_poll();
    bool is_send_queued();

    void disconnect_inactive_clients();
    void disconnect_client(ClientMap::iterator &it);

    void read_client_datagram();
    void parse_client_datagram(ssize_t size, sockaddr_storage *client_addr);
    void create_new_client(const std::string &addr_string,
                           session_id_t session_id,
                           uint8_t turn_direction,
                           const std::string &name,
                           sockaddr_storage *client_addr,
                           uint32_t next_expected);

    void send_datagram();
    bool send_to_client(EventsDatagram &datagram);

    GameServer(uint32_t port,
               uint32_t seed,
               uint32_t turning_speed,
               uint32_t rounds_per_sec,
               uint32_t width,
               uint32_t height);
  public:
    static GameServer create_from_program_arguments(int argc, char *argv[]);
    [[noreturn]] void run();
};
}

#endif //SIK2_SERVER_GAME_SERVER_H_
