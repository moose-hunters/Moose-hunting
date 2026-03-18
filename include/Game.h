#pragma once
#include "Window.h"

class Game {
private:
    Window m_window;
    bool m_running;

public:
    Game();
    ~Game() = default;

    void run();              // Главный игровой цикл
    void handleInput();      // Обработка ввода
    void update();           // Обновление логики
    void render();           // Отрисовка
};