#include "Game.h"
#include <iostream>

int main() {
    std::cout << "Starting MooseHunting..." << std::endl;

    Game game;

    if (!game.init(1024, 768, "Moose Hunting")) {
        std::cerr << "Initialization error!" << std::endl;
        return -1;
    }

    game.run();

    std::cout << "Bye!" << std::endl;
    return 0;
}