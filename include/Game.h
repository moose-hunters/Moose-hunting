#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Window.h"
#include "shader.h"
#include "Tree.h"
#include "Terrain.h"
#include <vector>
#include <enet/enet.h>
#include "Protocol.h"
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "miniaudio.h"

struct TreeInstance {
    glm::vec3 pos;
    int type;
    float scale;
};

struct BushInstance {
    glm::vec3 pos;
    float scale;
};

struct FenceInstance {
    glm::vec3 pos;
    float yaw;
    float scale;
};

enum class GameState {
    SPLASH,
    MENU,
    CONNECTING,
    PLAYING
};

struct Character {
    GLuint TextureID;      // ID текстуры глифа
    glm::ivec2 Size;       // Размер глифа
    glm::ivec2 Bearing;    // Смещение от базовой линии до левого/верхнего угла
    unsigned int Advance;  // Смещение до следующего символа
};

class Game {
   public:
    Game();
    ~Game();

    bool init(int width, int height, const char* title, const std::string& serverIP);
    void run();

   private:
    Window* m_window;
    Terrain m_terrain;
    int m_width, m_height;

    // Состояния игры
    GameState m_state = GameState::SPLASH;
    float m_stateTimer = 0.0f;
    std::string m_serverIP;
    EntityType m_selectedRole = EntityType::HUNTER;

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

    float m_floorY = 1.5f;

    // Деревья
    std::vector<Tree> m_treeModels;
    std::vector<TreeInstance> m_trees;

    // Кусты
    Tree m_bushModel;
    std::vector<BushInstance> m_bushes;

    // Лось
    Tree m_mooseModel;
    glm::vec3 m_moosePos;
    glm::vec2 m_mooseDir;
    float m_mooseSpeed = 2.0f;
    float m_mooseTimer = 0.0f;
    
    void updateMoose(float dt);

    Tree m_fenceModel;
    std::vector<FenceInstance> m_fences;

    // Физика прыжка
    float m_velocityY = 0.0f;
    bool m_isGrounded = true;
    const float m_gravity = -15.0f;
    const float m_jumpForce = 7.0f;
    const float m_cameraHeight = 3.5f; 

    // Стрельба
    Shader m_uiShader;
    GLuint m_uiVAO = 0, m_uiVBO = 0;

    float m_shootCooldown;
    float m_maxCooldown;
    bool m_wasLMBPressed;

    // Индикатор убийств
    int m_kills = 0;
    int m_deaths = 0;

    void setupUI();
    void renderUI();
    bool checkMooseHit();
    bool checkPlayerHit();

    GLuint m_gunVAO = 0, m_gunVBO = 0;
    std::vector<GLuint> m_gunFrames; 
    int m_currentGunFrame = 0;
    bool m_isShooting = false;
    float m_animationTimer = 0.0f;

    const float m_frameDuration = 0.08f;

    // Загрузка кадров
    Shader m_gunShader;
    GLuint loadTexture(const char* path);

    void renderGun();
    void updateGunAnimation(float dt);
    void setupGunUI();

    // Сеть
    ENetHost* m_clientHost = nullptr;
    ENetPeer* m_serverPeer = nullptr;
    int m_myId = 0;
    EntityType m_myRole = EntityType::HUNTER;

    struct EnemyState {
        EntityType role;
        glm::vec3 pos;
        float yaw;
        bool active = false;
    };

    EnemyState m_enemy;

    void processNetwork();

    // Отрисовка состояний
    void renderSplash();
    void renderMenu();
    void renderGame();

    Shader m_textShader;
    GLuint m_textVAO, m_textVBO;
    std::map<GLchar, Character> m_characters;

    bool initFont(const char* fontPath, unsigned int fontSize);
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color);

    ma_engine m_audioEngine;
    float m_stepTimer = 0.0f;
    float m_roarTimer = 25.0f;
};