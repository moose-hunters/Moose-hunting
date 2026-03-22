#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader() : m_program(0) {}

Shader::~Shader() {
    if (m_program) glDeleteProgram(m_program);
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    // работаем с вершинным шейдером
    std::string vertexCode;
    std::ifstream vFile(vertexPath);
    if (!vFile.is_open()) {
        std::cerr << "Failed to open vertex shader: " << vertexPath << std::endl;
        return false;
    }
    std::stringstream vStream;
    vStream << vFile.rdbuf();
    vertexCode = vStream.str();
    vFile.close();

    // Читаем фрагментный шейдер
    std::string fragmentCode;
    std::ifstream fFile(fragmentPath);
    if (!fFile.is_open()) {
        std::cerr << "Failed to open fragment shader: " << fragmentPath << std::endl;
        return false;
    }
    std::stringstream fStream;
    fStream << fFile.rdbuf();
    fragmentCode = fStream.str();
    fFile.close();

    // Компилируем шейдеры
    auto compileShader = [](GLenum type, const std::string& source) -> GLuint {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
        };

    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (!vertex || !fragment) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return false;
    }

    // Линкуем программу
    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        glDeleteProgram(m_program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}

void Shader::use() {
    glUseProgram(m_program);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) {
    glUniform3fv(glGetUniformLocation(m_program, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(m_program, name.c_str()), value);
}