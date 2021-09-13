#ifndef SIK2_COMMON_TIMER_POLL_H_
#define SIK2_COMMON_TIMER_POLL_H_

#include <poll.h>
#include <vector>

namespace screen_worms {
// A class responsible for polling over one normal socket and finite number of timers.
// (it was originally designed for up to two timers, so it was not tested with more of them)
// Listens both on IPv4 and IPv6.
class TimerPoll {
  private:
    std::vector<pollfd> poll_fds;

  public:
    // Argument sock_type is literally the "type" argument from socket().
    TimerPoll(uint32_t port, int sock_type, const std::vector<std::pair<long, long>> &times_s_and_ns);
    ~TimerPoll();

    // Argument timer_no is a number from 0 to n - 1 (as in the vector passed to the constructor).
    bool did_timer_expire(size_t timer_index);
    void reset_timer(size_t timer_index);
    bool is_client_ready_to_write();
    bool is_client_ready_to_read();
    int get_client_fd();
    void poll_blocking();
    void poll_blocking_with_pollout();
};
}
#endif //SIK2_COMMON_TIMER_POLL_H_