#define GLEW_STATIC
#include "Game.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static Game* g_gameInstance = nullptr;

Game::Game()
    : m_window(nullptr)
    , m_width(1024)
    , m_height(768)
    , m_cameraPos(0.0f, 5.0f, 15.0f)
    , m_cameraFront(0.0f, 0.0f, -1.0f)
    , m_cameraUp(0.0f, 1.0f, 0.0f)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_lastX(512.0f)
    , m_lastY(384.0f)
    , m_firstMouse(true)
    , m_lightPos(10.0f, 20.0f, 10.0f)
    , m_lightColor(1.0f, 1.0f, 1.0f)
{
    g_gameInstance = this;
}

Game::~Game() {
    cleanup();
}

bool Game::init(int width, int height, const char* title) {

    m_width = width;
    m_height = height;
    m_lastX = width / 2.0f;
    m_lastY = height / 2.0f;

    m_window = new Window(width, height, title);

    GLFWwindow* glfwWindow = m_window->getGLFWwindow();

    std::cout << "5. Checking OpenGL version..." << std::endl;
    const char* version = (const char*)glGetString(GL_VERSION);
    if (version) {
        std::cout << "OpenGL version: " << version << std::endl;
    }
    else {
        std::cerr << "error: glGetString returned NULL!" << std::endl;
        return false;
    }

    std::cout << "6. Setting up OpenGL..." << std::endl;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    std::cout << "7. Setting mouse input..." << std::endl;
    m_window->setCursorDisabled(true);
    m_window->setCursorPosCallback(mouseCallback);
    std::cout << "Cursor captured" << std::endl;

    std::cout << "8. Loading shaders..." << std::endl;
    if (!m_shader.load("shaders/tree_vertex.glsl", "shaders/tree_fragment.glsl")) {
        std::cerr << "error: failed to load shaders" << std::endl;
        return false;
    }

    std::cout << "9. Loading assets..." << std::endl;
    if (!m_tree.load("assets/tree.glb")) {
        std::cerr << "error: failed to load assets" << std::endl;
        return false;
    }

    std::cout << "Initialization completed successfully!" << std::endl;
    return true;
}

void Game::run() {
    float lastFrame = 0.0f;

    // обрабатываем кадры
    while (!m_window->shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        render();

        m_window->swapBuffers();
        m_window->pollEvents();
    }
}

void Game::processInput(float deltaTime) {
    float speed = 5.0f * deltaTime;
    GLFWwindow* glfwWindow = m_window->getGLFWwindow();

    // движение на wasd
    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += m_cameraFront * speed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= m_cameraFront * speed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;

    // выход через ESC
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfwWindow, true);
}

void Game::updateCamera() {
    // обновляем вектора направления камеры
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(front);
}

void Game::render() {
    m_window->clear();

    // обновляем камеру из мыши
    updateCamera();

    // создаем матрицу камеры, она определяет, куда мы смотрим
    glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_width / m_height, 0.1f, 100.0f);

    // используем шейдер
    m_shader.use();

    // передаем uniform'ы
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setVec3("lightPos", m_lightPos);
    m_shader.setVec3("lightColor", m_lightColor);
    m_shader.setVec3("viewPos", m_cameraPos);

    // рисуем дерево в позиции (0, 0, 0) с масштабом 1
    m_tree.render(view, projection, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void Game::cleanup() {
    m_tree.cleanup();
    if (m_window) {
        delete m_window;
        m_window = nullptr;
    }
}

// обработка движения мыши
void Game::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!g_gameInstance) return;

    if (g_gameInstance->m_firstMouse) {
        g_gameInstance->m_lastX = xpos;
        g_gameInstance->m_lastY = ypos;
        g_gameInstance->m_firstMouse = false;
    }

    float xoffset = xpos - g_gameInstance->m_lastX;
    float yoffset = g_gameInstance->m_lastY - ypos;
    g_gameInstance->m_lastX = xpos;
    g_gameInstance->m_lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    g_gameInstance->m_yaw += xoffset;
    g_gameInstance->m_pitch += yoffset;

    if (g_gameInstance->m_pitch > 89.0f) g_gameInstance->m_pitch = 89.0f;
    if (g_gameInstance->m_pitch < -89.0f) g_gameInstance->m_pitch = -89.0f;
}