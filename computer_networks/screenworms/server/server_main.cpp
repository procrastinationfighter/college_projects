#include "game_server.h"

int main(int argc, char *argv[]) {
    screen_worms::GameServer::create_from_program_arguments(argc, argv).run();
    return 0;
}
