CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -O2

.PHONY: all clean

all: screen-worms-server screen-worms-client

screen-worms-client: client/client_main.cpp
	$(CC) $(CFLAGS) -o $@ $^

screen-worms-server: server_main.o game_server.o game.o game_types.o utility.o timer_poll.o
	$(CC) $(CFLAGS) -o $@ $^

server_main.o: server/server_main.cpp server/game_server.h
	$(CC) $(CFLAGS) -c $<

game_server.o: server/game_server.cpp server/game_server.h server/game.h common/utility.h common/timer_poll.h
	$(CC) $(CFLAGS) -c $<

game.o: server/game.cpp server/game.h server/game_types.h
	$(CC) $(CFLAGS) -c $<

game_types.o: server/game_types.cpp server/game_types.h common/utility.h
	$(CC) $(CFLAGS) -c $<

timer_poll.o: common/timer_poll.cpp common/timer_poll.h common/utility.h
	$(CC) $(CFLAGS) -c $<

utility.o: common/utility.cpp common/utility.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o screen-worms-server screen-worms-client
