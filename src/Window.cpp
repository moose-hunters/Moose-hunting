#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title), m_window(nullptr) {

    std::cout << "1. GLFW initialization..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "error: failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "2. Window creating..." << std::endl;
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "error: failed to create window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    std::cout << "3. Making context current..." << std::endl;
    glfwMakeContextCurrent(m_window);

    std::cout << "4. GLAD initialization..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "error: failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << std::endl;
    glClearColor(42.0f / 255, 42.0f / 255, 53.0f / 255, 1.0f);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::setCursorDisabled(bool disabled) {
    if (disabled) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::setCursorPosCallback(GLFWcursorposfun callback) {
    glfwSetCursorPosCallback(m_window, callback);
}