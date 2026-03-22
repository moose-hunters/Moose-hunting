#include "Game.h"
#include <iostream>

int main() {
    std::cout << "Starting..." << std::endl;

    Game game;

    std::cout << "Calling init..." << std::endl;
    if (!game.init(1024, 768, "Moose Hunting")) {
        std::cerr << "initialization error!" << std::endl;
        return -1;
    }

    std::cout << "Starting the game loop..." << std::endl;
    game.run();

    std::cout << "end" << std::endl;
    return 0;
}