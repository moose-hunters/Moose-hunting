#include "Game.h"
#include <iostream>

int main() {
    std::cout << "=== НАЧАЛО ПРОГРАММЫ ===" << std::endl;

    Game game;

    std::cout << "Вызываем init..." << std::endl;
    if (!game.init(1024, 768, "Moose Hunting")) {
        std::cerr << "Ошибка инициализации!" << std::endl;
        return -1;
    }

    std::cout << "Запускаем игровой цикл..." << std::endl;
    game.run();

    std::cout << "=== ПРОГРАММА ЗАВЕРШЕНА ===" << std::endl;
    return 0;
}