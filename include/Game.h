#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Tree.h"
#include "shader.h"

class Game {
public:
    Game();
    ~Game();

    bool init(int width, int height, const char* title);
    void run();
    void cleanup();

private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;

    Tree m_tree;           // Одно дерево
    Shader m_shader;       // Шейдер для дерева

    // Камера (простейшая)
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;

    float m_yaw;
    float m_pitch;
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;

    // Освещение
    glm::vec3 m_lightPos;
    glm::vec3 m_lightColor;

    void processInput(float deltaTime);
    void updateCamera();
    void render();

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
};