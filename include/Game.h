#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Tree.h"
#include "shader.h"
#include "Window.h"

class Game {
public:
    Game();
    ~Game();

    bool init(int width, int height, const char* title);
    void run();
    void cleanup();

private:
    Window* m_window;
    int m_width;
    int m_height;

    // пока что только одно дерево
    Tree m_tree;
    Shader m_shader;
    
    // векторы для камеры
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;

    float m_yaw;
    float m_pitch;
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;

    // освещение
    glm::vec3 m_lightPos;
    glm::vec3 m_lightColor;

    void processInput(float deltaTime);
    void updateCamera();
    void render();

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
};