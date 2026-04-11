#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Window.h"
#include "shader.h"
#include "Tree.h"

class Game {
   public:
    Game();
    ~Game();

    bool init(int width, int height, const char* title);
    void run();

   private:
    Window* m_window;
    int m_width, m_height;

    // Камера
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    glm::vec3 m_cameraUp;
    float m_yaw;
    float m_pitch;
    float m_lastX, m_lastY;
    bool m_firstMouse;
    float m_fov;

    // Освещение
    glm::vec3 m_lightPos;
    glm::vec3 m_lightColor;

    Shader m_shader;
    Tree m_tree;

    // Пол
    GLuint m_floorVAO, m_floorVBO;
    GLuint m_floorTexture;
    void setupFloor();
    void renderFloor();

    void processInput(float deltaTime);
    void updateCamera();
    void render();
    void cleanup();

    static void mouseCallback(GLFWwindow* w, double x, double y);
    static void scrollCallback(GLFWwindow* w, double xoff, double yoff);

    // Ограничиваем Y камеры (ходим по полу)
    float m_floorY = 1.5f;
};