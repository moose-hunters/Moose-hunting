#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Window.h"
#include "shader.h"
#include "Tree.h"
#include "Terrain.h"
#include <vector>

struct TreeInstance {
    glm::vec3 pos;
    int type;
    float scale;
};

struct BushInstance {
    glm::vec3 pos;
    float scale;
};


class Game {
   public:
    Game();
    ~Game();

    bool init(int width, int height, const char* title);
    void run();

   private:
    Window* m_window;
    Terrain m_terrain;
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

    // Пол
    GLuint m_floorTexture;

    void processInput(float deltaTime);
    void updateCamera();
    void render();
    void cleanup();

    bool checkTreeCollision(glm::vec3 nextPos);

    static void mouseCallback(GLFWwindow* w, double x, double y);
    static void scrollCallback(GLFWwindow* w, double xoff, double yoff);

    // Ограничиваем Y камеры
    float m_floorY = 1.5f;

    // Деревья
    std::vector<Tree> m_treeModels;
    std::vector<TreeInstance> m_trees;

    // Кусты
    Tree m_bushModel;
    std::vector<BushInstance> m_bushes;

    // --- Лось ---
    Tree m_mooseModel;
    glm::vec3 m_moosePos;
    glm::vec2 m_mooseDir;      // Направление движения в плоскости XZ
    float m_mooseSpeed = 2.0f; // Скорость лося
    float m_mooseTimer = 0.0f; // Таймер до смены направления
    
    void updateMoose(float dt);


    // Физика прыжка
    float m_velocityY = 0.0f;
    bool m_isGrounded = true;
    const float m_gravity = -15.0f;
    const float m_jumpForce = 7.0f;
    const float m_cameraHeight = 1.5f; // Рост персонажа
};