#include "Game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static Game* g_gameInstance = nullptr;

Game::Game()
    : m_window(nullptr), m_width(1024), m_height(768), m_cameraPos(0.0f, 1.5f, 8.0f), m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f), m_yaw(-90.0f), m_pitch(0.0f), m_lastX(512.0f), m_lastY(384.0f), m_firstMouse(true), m_fov(45.0f), m_lightPos(5.0f, 10.0f, 5.0f), m_lightColor(1.0f, 0.95f, 0.85f), m_floorTexture(0), m_floorY(1.5f) {
    g_gameInstance = this;
}

Game::~Game() { cleanup(); }


// ------- collisions ---------
bool Game::checkTreeCollision(glm::vec3 nextPos) {
    const float playerRadius = 0.3f;
    const float treeRadius = 0.5f;
    const float minDist = playerRadius + treeRadius;

    // Теперь бегаем по структуре TreeInstance
    for (const auto& tree : m_trees) {
        float dx = nextPos.x - tree.pos.x;
        float dz = nextPos.z - tree.pos.z;
        if (dx * dx + dz * dz < minDist * minDist) {
            return true;
        }
    }
    // Кусты здесь НЕ проверяем, поэтому сквозь них можно ходить!
    return false;
}

// ---- Init ----
bool Game::init(int width, int height, const char* title) {
    m_width = width;
    m_height = height;
    m_lastX = width / 2.0f;
    m_lastY = height / 2.0f;

    m_window = new Window(width, height, title);
    GLFWwindow* w = m_window->getGLFWwindow();

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.35f, 0.55f, 0.75f, 1.0f);  // небо

    m_window->setCursorDisabled(true);
    m_window->setCursorPosCallback(mouseCallback);
    m_window->setScrollCallback(scrollCallback);

    // Шейдеры
    if (!m_shader.load("shaders/tree_vertex.glsl", "shaders/tree_fragment.glsl")) {
        std::cerr << "Failed to load shaders!" << std::endl;
        return false;
    }

    // Модель дерева — файл называется Tree1.glb
    m_treeModels.resize(3);
    m_treeModels[0].load("assets/Tree1.glb");
    m_treeModels[1].load("assets/Tree2.glb");
    m_treeModels[2].load("assets/Tree3.glb");

    // Загружаю куст
    m_bushModel.load("assets/Bush.glb");

    // Загружаю землю
    m_terrain.init("assets/Grass.png");

    std::cout << "Init OK. WASD = move, Mouse = look, Scroll = FOV, ESC = exit" << std::endl;

    for (int i = 0; i < 30; ++i) {
        float tx = (rand() % 4000 / 100.0f) - 20.0f;
        float tz = (rand() % 4000 / 100.0f) - 20.0f;
        float ty = m_terrain.getHeight(tx, tz);

        TreeInstance t;
        t.pos = glm::vec3(tx, ty, tz);
        t.type = rand() % 3; // Рандомно выбираем модель 0, 1 или 2
        m_trees.push_back(t);
    }

    for (int i = 0; i < 50; ++i) {
        float bx = (rand() % 4000 / 100.0f) - 20.0f;
        float bz = (rand() % 4000 / 100.0f) - 20.0f;
        float by = m_terrain.getHeight(bx, bz);
        m_bushes.push_back(glm::vec3(bx, by, bz));
    }

    // Стартовая позиция камеры на новой поверхности
    m_cameraPos.y = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;
    return true;
}

// ---- Loop ----
void Game::run() {
    float last = (float)glfwGetTime();
    while (!m_window->shouldClose()) {
        float now = (float)glfwGetTime();
        float dt = now - last;
        last = now;

        processInput(dt);
        render();

        m_window->swapBuffers();
        m_window->pollEvents();
    }
}

// ---- Input ----
void Game::processInput(float dt) {
    GLFWwindow* w = m_window->getGLFWwindow();

    // Ускорение
    float speed = (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 5.0f;
    speed *= dt;

    glm::vec3 flatFront = glm::normalize(glm::vec3(m_cameraFront.x, 0.0f, m_cameraFront.z));
    glm::vec3 right = glm::normalize(glm::cross(flatFront, m_cameraUp));

    auto moveWithCollision = [&](glm::vec3 direction) {
        glm::vec3 nextPos = m_cameraPos + direction;
        if (!checkTreeCollision(nextPos)) {
            m_cameraPos = nextPos;
        }
        };

    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) moveWithCollision(flatFront * speed);
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) moveWithCollision(-flatFront * speed);
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) moveWithCollision(-right * speed);
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) moveWithCollision(right * speed);

    // Прыжок и гравитация
    if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS && m_isGrounded) {
        m_velocityY = m_jumpForce;
        m_isGrounded = false;
    }
    m_velocityY += m_gravity * dt;
    m_cameraPos.y += m_velocityY * dt;

    // Высота берется строго из Terrain
    float groundY = m_terrain.getHeight(m_cameraPos.x, m_cameraPos.z) + m_cameraHeight;
    if (m_cameraPos.y <= groundY) {
        m_cameraPos.y = groundY;
        m_velocityY = 0.0f;
        m_isGrounded = true;
    }

    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(w, true);
}

// ---- Camera ----
void Game::updateCamera() {
    glm::vec3 front;
    front.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    front.y = sinf(glm::radians(m_pitch));
    front.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(front);
}

// ---- Render ----
void Game::render() {
    m_window->clear();
    updateCamera();

    glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    glm::mat4 proj = glm::perspective(glm::radians(m_fov),
        (float)m_width / (float)m_height,
        0.1f, 200.0f);

    m_shader.use();
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", proj);
    m_shader.setVec3("lightPos", m_lightPos);
    m_shader.setVec3("lightColor", m_lightColor);
    m_shader.setVec3("viewPos", m_cameraPos);
    m_shader.setFloat("ambientStrength", 0.35f);

    m_terrain.render(m_shader);

    // Рисую кусты
    for (const auto& pos : m_bushes) {
        m_bushModel.render(m_shader, pos, 3.0f);
    }

    // Рисуем деревья
    for (const auto& tree : m_trees) {
        // Берем модель нужного типа и рисуем ее по позиции
        m_treeModels[tree.type].render(m_shader, tree.pos, 1.0f);
    }
}

// ---- Cleanup ----
void Game::cleanup() {
    for (auto& tree : m_treeModels) {
        tree.cleanup();
    }
    m_bushModel.cleanup();

    if (m_floorTexture) {
        glDeleteTextures(1, &m_floorTexture);
        m_floorTexture = 0;
    }
    delete m_window;
    m_window = nullptr;
}

// ---- Callbacks ----
void Game::mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (!g_gameInstance) return;
    Game& g = *g_gameInstance;

    if (g.m_firstMouse) {
        g.m_lastX = (float)xpos;
        g.m_lastY = (float)ypos;
        g.m_firstMouse = false;
    }

    float dx = (float)xpos - g.m_lastX;
    float dy = g.m_lastY - (float)ypos;  // инвертируем Y
    g.m_lastX = (float)xpos;
    g.m_lastY = (float)ypos;

    const float sens = 0.1f;
    g.m_yaw += dx * sens;
    g.m_pitch += dy * sens;
    if (g.m_pitch > 89.0f) g.m_pitch = 89.0f;
    if (g.m_pitch < -89.0f) g.m_pitch = -89.0f;
}

void Game::scrollCallback(GLFWwindow*, double, double yoff) {
    if (!g_gameInstance) return;
    g_gameInstance->m_fov -= (float)yoff * 2.0f;
    if (g_gameInstance->m_fov < 10.0f) g_gameInstance->m_fov = 10.0f;
    if (g_gameInstance->m_fov > 90.0f) g_gameInstance->m_fov = 90.0f;
}