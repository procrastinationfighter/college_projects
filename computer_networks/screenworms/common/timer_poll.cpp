#include <sys/socket.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <climits>
#include <netdb.h>
#include <cstring>
#include <fcntl.h>

#include "timer_poll.h"
#include "utility.h"

namespace screen_worms {
TimerPoll::TimerPoll(uint32_t port, int sock_type, const std::vector<std::pair<long, long>> &times_s_and_ns)
                : poll_fds(times_s_and_ns.size() + 1) {
    // Methods used to initialize inspired by: https://beej.us/guide/bgnet/html/#datagram
    std::string port_string = std::to_string(port);
    struct addrinfo hints, *res;

    // Set main socket.
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = sock_type;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, port_string.c_str(), &hints, &res) < 0) {
        syserr("Getaddrinfo failed.");
    }

    while (res != nullptr) {
        poll_fds[0].fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (poll_fds[0].fd < 0) {
            print_log("ERROR: Failed to create socket.");
        } else if (bind(poll_fds[0].fd, res->ai_addr, res->ai_addrlen) < 0) {
            close(poll_fds[0].fd);
            print_log("ERROR: Failed to bind.");
        } else {
            break;
        }

        res = res->ai_next;
    }

    if (res == nullptr) {
        syserr("Failed to bind or create socket.");
    }

    freeaddrinfo(res);

    poll_fds[0].events = POLLIN;

    // Set timers.
    for (size_t i = 0; i < times_s_and_ns.size(); i++) {
        poll_fds[i + 1].fd = timerfd_create(CLOCK_MONOTONIC, 0);
        poll_fds[i + 1].events = POLLIN;

        if (poll_fds[i + 1].fd < 0) {
            syserr("Timer creation failed.");
        }

        struct itimerspec timerspec {
            {times_s_and_ns[i].first, times_s_and_ns[i].second},
            {times_s_and_ns[i].first, times_s_and_ns[i].second}
        };

        if (timerfd_settime(poll_fds[i + 1].fd, NO_FLAGS, &timerspec, nullptr) < 0) {
            syserr("Setting timer failed.");
        }
    }

    if (sock_type == SOCK_DGRAM) {
        fcntl(poll_fds[0].fd, F_SETFL, O_NONBLOCK);
    }

    print_log("Poll is ready to receive on port " << port);
}

TimerPoll::~TimerPoll() {
    for (auto &desc : poll_fds) {
        if (desc.fd >= 0 && close(desc.fd)) {
            syserr("Error while closing poll descriptor " << desc.fd);
        }
    }
}

bool TimerPoll::did_timer_expire(size_t timer_index) {
    return (poll_fds[timer_index + 1].revents & POLLIN);
}

void TimerPoll::reset_timer(size_t timer_index) {
    uint64_t timer = 0;
    read(poll_fds[timer_index + 1].fd, &timer, 8);
}

bool TimerPoll::is_client_ready_to_write() {
    return (poll_fds[0].revents & POLLIN);
}

bool TimerPoll::is_client_ready_to_read() {
    return (poll_fds[0].revents & POLLOUT);
}

int TimerPoll::get_client_fd() {
    return poll_fds[0].fd;
}

void TimerPoll::poll_blocking() {
    poll_fds[0].events = POLLIN;
    int res = poll(poll_fds.data(), poll_fds.size(), -1);

    if (res < 0) {
        syserr("Poll failed.");
    } else if (res == 0) {
        syserr("Empty poll");
    }
}

void TimerPoll::poll_blocking_with_pollout() {
    poll_fds[0].events |= POLLOUT;
    int res = poll(poll_fds.data(), poll_fds.size(), -1);

    if (res < 0) {
        syserr("Poll failed.");
    } else if (res == 0) {
        syserr("Empty poll");
    }
}
}