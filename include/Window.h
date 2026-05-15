#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window {
   public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void clear();
    void setCursorDisabled(bool disabled);
    void setCursorPosCallback(GLFWcursorposfun callback);
    void setScrollCallback(GLFWscrollfun callback);

    GLFWwindow* getGLFWwindow() const { return m_window; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

   private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    std::string m_title;
};