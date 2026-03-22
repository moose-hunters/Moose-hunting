#define GLEW_STATIC
#include "Game.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Статический указатель для callback
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

    std::cout << "1. Инициализация GLFW..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "Ошибка: не удалось инициализировать GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "2. Создание окна..." << std::endl;
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Ошибка: не удалось создать окно" << std::endl;
        glfwTerminate();
        return false;
    }

    std::cout << "3. Делаем контекст текущим..." << std::endl;
    glfwMakeContextCurrent(m_window);

    std::cout << "4. Инициализация GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Ошибка: не удалось инициализировать GLAD" << std::endl;
        return false;
    }

    std::cout << "5. Проверка OpenGL версии..." << std::endl;
    const char* version = (const char*)glGetString(GL_VERSION);
    if (version) {
        std::cout << "OpenGL version: " << version << std::endl;
    }
    else {
        std::cerr << "Ошибка: glGetString вернул NULL!" << std::endl;
        return false;
    }

    std::cout << "6. Настройка OpenGL..." << std::endl;
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    std::cout << "7. Загрузка шейдера..." << std::endl;
    if (!m_shader.load("shaders/tree_vertex.glsl", "shaders/tree_fragment.glsl")) {
        std::cerr << "Ошибка: не удалось загрузить шейдер" << std::endl;
        return false;
    }

    std::cout << "8. Загрузка дерева..." << std::endl;
    if (!m_tree.load("assets/tree.glb")) {
        std::cerr << "Ошибка: не удалось загрузить модель дерева" << std::endl;
        return false;
    }

    std::cout << "Инициализация завершена успешно!" << std::endl;
    return true;
}

void Game::run() {
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(m_window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(deltaTime);

        render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Game::processInput(float deltaTime) {
    float speed = 5.0f * deltaTime;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += m_cameraFront * speed;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= m_cameraFront * speed;
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * speed;

    // ESC для выхода
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);
}

void Game::updateCamera() {
    // Обновление вектора направления камеры из yaw/pitch
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_cameraFront = glm::normalize(front);
}

void Game::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Обновляем камеру из мыши
    updateCamera();

    // Матрицы
    glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_width / m_height, 0.1f, 100.0f);

    // Используем шейдер
    m_shader.use();

    // Передаем uniform'ы
    m_shader.setVec3("lightPos", m_lightPos);
    m_shader.setVec3("lightColor", m_lightColor);
    m_shader.setVec3("viewPos", m_cameraPos);

    // Рисуем дерево в позиции (0, 0, 0) с масштабом 1
    m_tree.render(view, projection, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void Game::cleanup() {
    m_tree.cleanup();
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

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