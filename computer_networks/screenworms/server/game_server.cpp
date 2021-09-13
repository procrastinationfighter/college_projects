#include "game_server.h"
#include <ctime>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <utility>

namespace {
constexpr int ARG_TYPES_COUNT = 6;
const char *ARG_SYMBOLS = "+p:s:t:v:w:h:";
constexpr char PORT_SYMBOL = 'p',
        SEED_SYMBOL = 's',
        TURNING_SPEED_SYMBOL = 't',
        ROUNDS_PER_SEC_SYMBOL = 'v',
        BOARD_WIDTH_SYMBOL = 'w',
        BOARD_HEIGHT_SYMBOL = 'h';

constexpr int PORT_ID = 0,
        SEED_ID = 1,
        TURNING_SPEED_ID = 2,
        ROUNDS_PER_SEC_ID = 3,
        BOARD_WIDTH_ID = 4,
        BOARD_HEIGHT_ID = 5;

// Default program parameters.
constexpr int DEFAULT_PORT = 2021;
constexpr int DEFAULT_TURNING_SPEED = 6;
constexpr int DEFAULT_ROUNDS_PER_SEC = 50;
constexpr int DEFAULT_BOARD_WIDTH = 640;
constexpr int DEFAULT_BOARD_HEIGHT = 480;

// Parameters limits.
constexpr int MAXIMAL_PORT = 65535;
constexpr int MINIMAL_PORT = 1024;
constexpr uint32_t MIN_SEED = 1;
constexpr uint32_t MAX_SEED = UINT32_MAX;
constexpr int MIN_TURNING_SPEED = 1;
constexpr int MAX_TURNING_SPEED = 90;
constexpr int MIN_ROUNDS_PER_SEC = 1;
constexpr int MAX_ROUNDS_PER_SEC = 250;
constexpr int MIN_BOARD_WIDTH = 16;
constexpr int MAX_BOARD_WIDTH = 4096;
constexpr int MIN_BOARD_HEIGHT = 16;
constexpr int MAX_BOARD_HEIGHT = 4096;

// Timer parameters.
constexpr int CLIENT_TIMEOUT_S = 0;
constexpr uint32_t CLIENT_TIMEOUT_NS = 250000000;

constexpr int CLIENT_TIMEOUT_INDEX = 0;
constexpr int ROUND_TIMEOUT_INDEX = 1;

constexpr int NS_IN_S = 1000000000;

// Time in milliseconds after which clients should be disconnected.
constexpr int CLIENT_TIMEOUT_TIME_MS = 2000;

// Byte limit for one datagram.
constexpr int DATAGRAM_MAX_LEN = 550;

// Separator for address strings.
constexpr char SOCKET_STR_SEPARATOR = '/';

// Character limits for a name.
constexpr char NAME_MINIMAL_CHAR = 33;
constexpr char NAME_MAXIMAL_CHAR = 126;

constexpr int MINIMAL_PLAYERS_COUNT = 2;

constexpr int MAX_CLIENTS = 25;

uint32_t get_default_seed() {
    // Return last 32 bits.
    return (time(nullptr) & 0xffffffff);
}

void get_arguments(int argc, char *argv[], char *arguments[]) {
    int opt;
    while ((opt = getopt(argc, argv, ARG_SYMBOLS)) != -1) {
        switch (opt) {
            case PORT_SYMBOL:
                arguments[PORT_ID] = optarg;
                break;
            case SEED_SYMBOL:
                arguments[SEED_ID] = optarg;
                break;
            case TURNING_SPEED_SYMBOL:
                arguments[TURNING_SPEED_ID] = optarg;
                break;
            case ROUNDS_PER_SEC_SYMBOL:
                arguments[ROUNDS_PER_SEC_ID] = optarg;
                break;
            case BOARD_WIDTH_SYMBOL:
                arguments[BOARD_WIDTH_ID] = optarg;
                break;
            case BOARD_HEIGHT_SYMBOL:
                arguments[BOARD_HEIGHT_ID] = optarg;
                break;
            default:
            syserr("Unexpected program argument.");
        }
    }
    if (optind != argc) {
        syserr("Unexpected program argument: " << argv[optind]);
    }
}

void init_arg_values(uint32_t arg_values[]) {
    arg_values[PORT_ID] = DEFAULT_PORT;
    arg_values[SEED_ID] = get_default_seed();
    arg_values[TURNING_SPEED_ID] = DEFAULT_TURNING_SPEED;
    arg_values[ROUNDS_PER_SEC_ID] = DEFAULT_ROUNDS_PER_SEC;
    arg_values[BOARD_WIDTH_ID] = DEFAULT_BOARD_WIDTH;
    arg_values[BOARD_HEIGHT_ID] = DEFAULT_BOARD_HEIGHT;
}

uint32_t parse_argument(const std::string &argument) {
    try {
        size_t len;
        long arg = std::stol(argument, &len);

        if (len == argument.length()) {
            if (arg > UINT32_MAX || arg <= 0) {
                syserr("Argument out of range (32 bits unsigned positive): " << argument);
            } else {
                return arg;
            }
        } else {
            throw std::invalid_argument("");
        }
    } catch (std::invalid_argument &e) {
        syserr("Wrong argument: " << argument);
    }
}

void validate_port(uint32_t port) {
    if (port > MAXIMAL_PORT || port < MINIMAL_PORT) {
        syserr("Wrong port value.");
    }
}

void validate_seed(uint32_t seed) {
    if (seed > MAX_SEED || seed < MIN_SEED) {
        syserr("Wrong seed value.");
    }
}

void validate_turning_speed(uint32_t speed) {
    if (speed > MAX_TURNING_SPEED || speed < MIN_TURNING_SPEED) {
        syserr("Wrong turning speed value.");
    }
}

void validate_rounds(uint32_t rounds) {
    if (rounds > MAX_ROUNDS_PER_SEC || rounds < MIN_ROUNDS_PER_SEC) {
        syserr("Wrong rounds per sec value.");
    }
}

void validate_width(uint32_t width) {
    if (width > MAX_BOARD_WIDTH || width < MIN_BOARD_WIDTH) {
        syserr("Wrong board width value.");
    }
}

void validate_height(uint32_t height) {
    if (height > MAX_BOARD_HEIGHT || height < MIN_BOARD_HEIGHT) {
        syserr("Wrong board height value.");
    }
}

void validate_argument_values(const uint32_t values[]) {
    validate_port(values[PORT_ID]);
    validate_seed(values[SEED_ID]);
    validate_turning_speed(values[TURNING_SPEED_ID]);
    validate_rounds(values[ROUNDS_PER_SEC_ID]);
    validate_width(values[BOARD_WIDTH_ID]);
    validate_height(values[BOARD_HEIGHT_ID]);
}

// Given rounds_per_sec parameter, returns pair {seconds, nanoseconds}
// describing the length of the interval between rounds.
std::pair<long, long> get_rounds_timer_data(uint32_t rounds_per_sec) {
    if (rounds_per_sec == 1) {
        return {1, 0};
    } else {
        // Division by 0 doesn't occur - rounds_per_sec can't be 0.
        double sec_delay = 1.0 / rounds_per_sec;
        return {0, NS_IN_S * sec_delay};
    }
}

std::string get_client_socket_str(struct sockaddr_storage *sa) {
    void *address;
    in_port_t port;
    static char s[INET6_ADDRSTRLEN];

    if (((struct sockaddr *) sa)->sa_family == AF_INET) {
        address = (void *) &(((struct sockaddr_in *) sa)->sin_addr);
        port = ((struct sockaddr_in *) (sa))->sin_port;
    } else {
        address = (void *) &(((struct sockaddr_in6 *) sa)->sin6_addr);
        port = ((struct sockaddr_in6 *) (sa))->sin6_port;
    }

    return std::move(std::string(inet_ntop(sa->ss_family, address, s, sizeof(s)))
                             + SOCKET_STR_SEPARATOR
                             + std::to_string(port));
}

bool is_name_valid(const std::string &name) {
    for (char ch : name) {
        if (ch < NAME_MINIMAL_CHAR || ch > NAME_MAXIMAL_CHAR) {
            return false;
        }
    }

    return true;
}

uint32_t get_first_datagram_id_by_event_id(const std::vector<screen_worms::EventsDatagram> &datagrams,
                                           uint32_t first_event_id) {
    // Using binary search, find the datagram with desired event.
    // Assumes that the value can be found.
    uint32_t left = 0, right = datagrams.size() - 1, mid;
    while (left < right) {
        mid = (left + right) / 2;
        auto bounds = datagrams[mid].get_bounds();
        if (first_event_id > bounds.second) {
            left = mid + 1;
        } else if (first_event_id < bounds.first) {
            right = mid - 1;
        } else {
            return mid;
        }
    }

    return left;
}
}

namespace screen_worms {
GameServer::GameServer(uint32_t port,
                       uint32_t seed,
                       uint32_t turning_speed,
                       uint32_t rounds_per_sec,
                       uint32_t width,
                       uint32_t height)
        : timer_poll(port,
                     SOCK_DGRAM,
                     {{CLIENT_TIMEOUT_S, CLIENT_TIMEOUT_NS},
                      get_rounds_timer_data(rounds_per_sec)}),
          game(seed, turning_speed, width, height),
          next_datagram_target(clients.end()),
          not_ready_players(0) {}

GameServer GameServer::create_from_program_arguments(int argc, char **argv) {
    // Mapping from array elements to arguments is given by constants.
    char *arguments[ARG_TYPES_COUNT];
    for (auto &argument : arguments) {
        argument = nullptr;
    }

    uint32_t arg_values[ARG_TYPES_COUNT];
    init_arg_values(arg_values);

    get_arguments(argc, argv, arguments);

    for (int i = 0; i < ARG_TYPES_COUNT; i++) {
        if (arguments[i] != nullptr) {
            arg_values[i] = parse_argument(arguments[i]);
        }
    }

    validate_argument_values(arg_values);

    // Casting longs to ints is OK, because they were already validated.
    return screen_worms::GameServer(arg_values[PORT_ID],
                                    arg_values[SEED_ID],
                                    arg_values[TURNING_SPEED_ID],
                                    arg_values[ROUNDS_PER_SEC_ID],
                                    arg_values[BOARD_WIDTH_ID],
                                    arg_values[BOARD_HEIGHT_ID]);
}

[[noreturn]] void GameServer::run() {
    // Constructor should have initialized all dependencies, so we can just use them.
    while (true) {
        do_poll();

        if (timer_poll.did_timer_expire(ROUND_TIMEOUT_INDEX)) {
            // Play a round or start a new game.
            timer_poll.reset_timer(ROUND_TIMEOUT_INDEX);

            if (game.is_game_active()) {
                if (!game.play_round()) {
                    // The game has finished. Now wait for players to prepare for a new one.
                    // We wait for number of players equal to the number of taken names.
                    not_ready_players = static_cast<int>(taken_names.size());
                    for (auto &[_, client] : clients) {
                        client.reset_ready();
                    }
                }
            } else {
                // Check if you can start game, if yes, do so.
                if (not_ready_players == 0 && taken_names.size() >= MINIMAL_PLAYERS_COUNT) {
                    game.create_new_game();
                }
            }
        }
        if (timer_poll.did_timer_expire(CLIENT_TIMEOUT_INDEX)) {
            timer_poll.reset_timer(CLIENT_TIMEOUT_INDEX);
            disconnect_inactive_clients();
        }
        if (timer_poll.is_client_ready_to_write()) {
            read_client_datagram();
        }
        if (timer_poll.is_client_ready_to_read()) {
            // Pick a right datagram target and send them their desired datagram.
            send_datagram();
        }
    }
}

void GameServer::set_client_ready_if_possible(Client &client, uint32_t turn_direction) {
    if (!game.is_game_active() && turn_direction != TURN_DIRECTION_STRAIGHT
            && !client.is_ready_to_play()) {
        not_ready_players--;
        client.set_ready();
    }
}

void GameServer::do_poll() {
    // If we can send something, poll with POLLOUT.
    if (is_send_queued()) {
        timer_poll.poll_blocking_with_pollout();
    } else {
        timer_poll.poll_blocking();
    }
}

bool GameServer::is_send_queued() {
    auto datagrams = game.get_datagrams();
    if (datagrams.empty()) {
        return false;
    }

    // Return true if any client would like to get a datagram.
    uint32_t last_event = datagrams.back().get_bounds().second;

    for (auto &[_, client] : clients) {
        if (client.get_next_event_expected() <= last_event) {
            return true;
        }
    }

    return false;
}

void GameServer::disconnect_inactive_clients() {
    auto time_point = std::chrono::system_clock::now();
    bool del = false;
    ClientMap::iterator prev;

    for (auto client_it = clients.begin(); client_it != clients.end();
         client_it = std::next(client_it)) {
        if (del) {
            disconnect_client(prev);
        }

        del = client_it->second.should_get_timeout(time_point);
        prev = client_it;
    }

    if (del) {
        disconnect_client(prev);
    }
}

void GameServer::disconnect_client(ClientMap::iterator &it) {
    game.delete_player(it->second.get_name());

    // If the player was not ready to play, decrease the counter of not ready players.
    if (!game.is_game_active() && !it->second.is_ready_to_play()) {
        not_ready_players--;
    }
    taken_names.erase(it->second.get_name());
    if (next_datagram_target == it) {
        next_datagram_target = std::next(next_datagram_target);
    }
    clients.erase(it);
}

void GameServer::read_client_datagram() {
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);

    ssize_t received = recvfrom(timer_poll.get_client_fd(),
                                datagram_buffer,
                                CLIENT_DATAGRAM_MAX_LEN,
                                NO_FLAGS,
                                (struct sockaddr *) &client_addr,
                                &addr_len);
    if (received < 0) {
        print_log("Error while receiving a datagram.");
        return;
    } else if (received < CLIENT_DATAGRAM_MIN_LEN) {
        // Ignore bad datagram.
        return;
    } else {
        parse_client_datagram(received, &client_addr);
    }
}

void GameServer::parse_client_datagram(ssize_t size, sockaddr_storage *client_addr) {
    session_id_t session_id;
    uint8_t turn_direction;
    uint32_t next_expected;

    // Name can contain 0-20 characters.
    ssize_t name_len = size - CLIENT_DATAGRAM_MIN_LEN;
    std::string name;
    name.resize(name_len);

    memcpy(&session_id, datagram_buffer, sizeof(session_id));
    turn_direction = static_cast<uint8_t>(datagram_buffer[8]);
    memcpy(&next_expected, datagram_buffer + 9, sizeof(next_expected));

    session_id = be64toh(session_id);
    next_expected = ntohl(next_expected);

    if (name_len > 0) {
        memcpy(name.data(), datagram_buffer + 13, name_len);
    }

    std::string addr_string = get_client_socket_str(client_addr);

    auto client_it = clients.find(addr_string);
    if (client_it == clients.end()) {
        // Client with given address not found.
        if (is_name_valid(name) && taken_names.find(name) == taken_names.end()
                && clients.size() < MAX_CLIENTS) {
            create_new_client(addr_string,
                              session_id,
                              turn_direction,
                              name,
                              client_addr,
                              next_expected);
        } else {
            // Name contains illegal characters or is taken or client limit reached. Ignore.
            return;
        }
    } else {
        if (client_it->second.is_session_id_greater(session_id)) {
            // If a client with greater session id connected,
            // severe the previous connection and create a new one.
            disconnect_client(client_it);
            create_new_client(addr_string,
                              session_id,
                              turn_direction,
                              name,
                              client_addr,
                              next_expected);
        } else if (client_it->second.is_session_id_equal(session_id)
                && client_it->second.are_names_equal(name)) {
            // The "normal" handling of a previously connected client.
            game.change_player_turning_direction(name, turn_direction);
            client_it->second.set_next_event_expected(next_expected);
            client_it->second.update_contact();

            set_client_ready_if_possible(client_it->second, turn_direction);
        } else {
            // Ignore datagram with smaller session id or different name than before.
            return;
        }
    }
}

void GameServer::create_new_client(const std::string &addr_string,
                                   session_id_t session_id,
                                   uint8_t turn_direction,
                                   const std::string &name,
                                   sockaddr_storage *client_addr,
                                   uint32_t next_expected) {
    auto client = clients.insert({addr_string, Client(session_id, name, client_addr)});
    if (!client.second) {
        // Adding failed.
        print_log("Error: Adding client " << session_id << " with name " << name);
        return;
    }
    client.first->second.set_next_event_expected(next_expected);
    client.first->second.update_contact();

    if (!client.first->second.is_spectator()) {
        if (!game.is_game_active()) {
            not_ready_players++;
        }

        auto res = taken_names.insert(name);
        if (!res.second) {
            print_log("Error: Adding name " << name << " to the pool of taken names");
        }
        set_client_ready_if_possible(client.first->second, turn_direction);

        game.add_new_player(name);
        game.change_player_turning_direction(name, turn_direction);
    }
}

void GameServer::send_datagram() {
    if (!clients.empty()) {
        if (next_datagram_target == clients.end()) {
            next_datagram_target = clients.begin();
        }

        auto datagrams = game.get_datagrams();

        // If we get to this point, there MUST BE a client that we can send a datagram to.
        // Find the first one that expect an existing event.
        uint32_t last_event = datagrams.back().get_bounds().second;
        while (next_datagram_target->second.get_next_event_expected() > last_event) {
            next_datagram_target = std::next(next_datagram_target);
            if (next_datagram_target == clients.end()) {
                next_datagram_target = clients.begin();
            }
        }
        uint32_t datagram_index =
                get_first_datagram_id_by_event_id(datagrams,
                                                  next_datagram_target->second.get_next_event_expected());
        EventsDatagram &datagram = datagrams[datagram_index];
        // Set next expected to the first one that wasn't sent.
        next_datagram_target->second.set_next_event_expected(datagram.get_bounds().second + 1);
        if (send_to_client(datagram)) {
            // If send was not successful, we will try to retransmit it later.
            next_datagram_target = std::next(next_datagram_target);
        }
    }
}

bool GameServer::send_to_client(EventsDatagram &datagram) {
    auto res = sendto(timer_poll.get_client_fd(), datagram.get_datagram(),
                      datagram.get_datagram_size(), NO_FLAGS,
                      (struct sockaddr *) next_datagram_target->second.get_address_ptr(),
                      next_datagram_target->second.get_address_size());
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return false;
    } else if (res < 0) {
        print_log("Send utterly failed");
    }
    return true;
}

Client::Client(session_id_t session_id, std::string nickname, const sockaddr_storage *addr)
        : id(session_id), name(std::move(nickname)), is_ready(false), address(*addr) {}

bool Client::is_spectator() const {
    return name.empty();
}

void Client::set_ready() {
    is_ready = true;
}

void Client::reset_ready() {
    is_ready = false;
}

void Client::update_contact() {
    last_contact = std::chrono::system_clock::now();
}

bool Client::should_get_timeout(const std::chrono::time_point<std::chrono::system_clock> &now) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - last_contact).count()
            >= CLIENT_TIMEOUT_TIME_MS;
}

bool Client::are_names_equal(const std::string &name) const {
    return this->name == name;
}

bool Client::is_session_id_greater(session_id_t id) const {
    return this->id > id;
}

bool Client::is_session_id_equal(session_id_t id) const {
    return this->id == id;
}

const std::string &Client::get_name() const {
    return name;
}

bool Client::is_ready_to_play() const {
    return is_ready;
}

void Client::set_next_event_expected(uint32_t id) {
    next_event_expected = id;
}

uint32_t Client::get_next_event_expected() const {
    return next_event_expected;
}

struct sockaddr_storage *Client::get_address_ptr() {
    return &address;
}

socklen_t Client::get_address_size() {
    return sizeof(address);
}
}