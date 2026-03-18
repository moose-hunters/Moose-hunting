#include "Game.h"

Game::Game()
    : m_window(640, 480, "Hello World"), m_running(true) {
    // Здесь можно добавить инициализацию игры
    // (загрузка текстур, создание объектов и т.д.)
}

void Game::run() {
    while (!m_window.shouldClose() && m_running) {
        handleInput();
        update();
        render();

        m_window.swapBuffers();
        m_window.pollEvents();
    }
}

void Game::handleInput() {
    // Здесь будет обработка нажатий клавиш
    // Например, если нажали ESC - выйти
    // (пока оставим пустым)
}

void Game::update() {
    // Здесь будет обновление логики
    // (движение объектов, физика и т.д.)
}

void Game::render() {
    // Очищаем экран
    m_window.clear();

    // Здесь будет отрисовка объектов
    // (пока ничего не рисуем)
}