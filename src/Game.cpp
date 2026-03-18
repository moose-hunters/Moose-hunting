#include <windows.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Game.h"
#include "Cube.h"

// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

Game::Game()
    : m_window(640, 480, "Moose hunting"), m_running(true), m_rotation(0.0f) {
    setupShaders();
}

unsigned int Game::compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Проверка ошибок
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

void Game::setupShaders() {
    // Компилируем шейдеры
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Создаем программу
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Проверка линковки
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
    }

    // Можно удалить шейдеры после линковки
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
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
    GLFWwindow* window = m_window.getNativeWindow();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        m_running = false;
    }
}

void Game::update() {
    m_rotation += 1.0f;
    if (m_rotation > 360.0f) m_rotation -= 360.0f;
}

void Game::render() {
    m_window.clear();
    glEnable(GL_DEPTH_TEST);

    // Создаем матрицы с помощью GLM
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)m_window.getWidth() / (float)m_window.getHeight(),
        0.1f, 100.0f
    );

    glm::mat4 view = glm::lookAt(
        glm::vec3(3.0f, 3.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 model = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(m_rotation),
        glm::vec3(1.0f, 1.0f, 0.0f)
    );

    glm::mat4 mvp = projection * view * model;

    // Используем шейдер и передаем матрицу
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE,
        glm::value_ptr(mvp));

    m_cube.render();
}