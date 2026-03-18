#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window {
private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    std::string m_title;

public:
    // Конструктор и деструктор
    Window(int width, int height, const std::string& title);
    ~Window();

    // Запрещаем копирование (чтобы случайно не размножить окно)
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Основные методы
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void clear();

    // Геттеры
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getNativeWindow() const { return m_window; }
};