#include "game/Game.h"

int main(int, char**) {
    Game game;
    if (!game.init()) { game.shutdown(); return 1; }
    game.run();
    game.shutdown();
    return 0;
}