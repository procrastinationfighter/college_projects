#include "userInterface.h"

int main() {
    World world;
    initializeWorld(&world);
    manageWorld(&world);
    deinitializeWorld(&world);
    return 0;
}
