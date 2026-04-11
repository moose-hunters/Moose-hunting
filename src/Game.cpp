#include "Game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static Game* g_gameInstance = nullptr;

Game::Game()
    : m_window(nullptr), m_width(1024), m_height(768), m_cameraPos(0.0f, 1.5f, 8.0f), m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f), m_yaw(-90.0f), m_pitch(0.0f), m_lastX(512.0f), m_lastY(384.0f), m_firstMouse(true), m_fov(45.0f), m_lightPos(5.0f, 10.0f, 5.0f), m_lightColor(1.0f, 0.95f, 0.85f), m_floorVAO(0), m_floorVBO(0), m_floorTexture(0), m_floorY(1.5f) {
    g_gameInstance = this;
}

Game::~Game() { cleanup(); }

// ---- Пол ----
void Game::setupFloor() {
    // Большая плоская плоскость 40x40
    float s = 20.0f;
    float y = 0.0f;
    float t = 10.0f;  // тайлинг текстуры
    float verts[] = {
        //  x     y   z      nx   ny   nz    u    v
        -s,
        y,
        -s,
        0,
        1,
        0,
        0,
        0,
        s,
        y,
        -s,
        0,
        1,
        0,
        t,
        0,
        s,
        y,
        s,
        0,
        1,
        0,
        t,
        t,
        s,
        y,
        s,
        0,
        1,
        0,
        t,
        t,
        -s,
        y,
        s,
        0,
        1,
        0,
        0,
        t,
        -s,
        y,
        -s,
        0,
        1,
        0,
        0,
        0,
    };
    glGenVertexArrays(1, &m_floorVAO);
    glGenBuffers(1, &m_floorVBO);
    glBindVertexArray(m_floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // texCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    // Простая шахматная текстура 64x64
    glGenTextures(1, &m_floorTexture);
    glBindTexture(GL_TEXTURE_2D, m_floorTexture);
    const int SZ = 64;
    unsigned char pixels[SZ * SZ * 3];
    for (int py = 0; py < SZ; py++) {
        for (int px = 0; px < SZ; px++) {
            bool w = ((px / 8 + py / 8) % 2) == 0;
            int idx = (py * SZ + px) * 3;
            pixels[idx + 0] = w ? 100 : 60;
            pixels[idx + 1] = w ? 120 : 80;
            pixels[idx + 2] = w ? 80 : 50;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SZ, SZ, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Game::renderFloor() {
    glm::mat4 model = glm::mat4(1.0f);
    m_shader.setMat4("model", model);
    m_shader.setMat3("normalMatrix", glm::mat3(1.0f));
    m_shader.setVec4("baseColorFactor", glm::vec4(1.0f));
    m_shader.setBool("hasTexture", true);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_floorTexture);
    m_shader.setInt("texture_diffuse0", 0);

    glBindVertexArray(m_floorVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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
    if (!m_tree.load("assets/Tree1.glb")) {
        std::cerr << "Failed to load Tree1.glb!" << std::endl;
        return false;
    }

    setupFloor();
    std::cout << "Init OK. WASD = move, Mouse = look, Scroll = FOV, ESC = exit" << std::endl;
    return true;
}

// ---- Loop ----
void Game::run() {
    float last = 0.0f;
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
    float speed = 5.0f * dt;

    // Горизонтальное движение — вектор без Y
    glm::vec3 flatFront = glm::normalize(glm::vec3(m_cameraFront.x, 0.0f, m_cameraFront.z));
    glm::vec3 right = glm::normalize(glm::cross(flatFront, m_cameraUp));

    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) m_cameraPos += flatFront * speed;
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) m_cameraPos -= flatFront * speed;
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) m_cameraPos -= right * speed;
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) m_cameraPos += right * speed;

    // Камера всегда на высоте пола (FPS-стиль)
    m_cameraPos.y = m_floorY;

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

    // Рисуем пол
    renderFloor();

    // Рисуем дерево: позиция (0,0,0), масштаб подберите под вашу модель
    m_tree.render(m_shader, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

// ---- Cleanup ----
void Game::cleanup() {
    m_tree.cleanup();
    if (m_floorVAO) {
        glDeleteVertexArrays(1, &m_floorVAO);
        m_floorVAO = 0;
    }
    if (m_floorVBO) {
        glDeleteBuffers(1, &m_floorVBO);
        m_floorVBO = 0;
    }
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