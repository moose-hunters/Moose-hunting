#pragma once

#include "Window.h"
#include "Cube.h"

class Game {
private:
    Window m_window;
    Cube m_cube;
    bool m_running;
    float m_rotation;

    // ID шейдерной программы
    unsigned int shaderProgram;

    // Вспомогательные функции
    unsigned int compileShader(const char* source, GLenum type);
    void setupShaders();

public:
    Game();
    ~Game() = default;

    void run();
    void handleInput();
    void update();
    void render();
};