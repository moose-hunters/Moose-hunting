#include "gl_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

void checkGLError(const std::string& location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << location << ": " << err << std::endl;
    }
}

GLuint compileShader(GLenum type, const std::string& source) {
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
}

GLuint createProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    // Читаем вершинный шейдер
    std::string vertexCode;
    std::ifstream vFile(vertexPath);
    if (!vFile.is_open()) {
        std::cerr << "Failed to open vertex shader: " << vertexPath << std::endl;
        return 0;
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
        return 0;
    }
    std::stringstream fStream;
    fStream << fFile.rdbuf();
    fragmentCode = fStream.str();
    fFile.close();

    // Компилируем шейдеры
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (!vertex || !fragment) {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return 0;
    }

    // Линкуем программу
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

GLuint createVAO(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    GLuint VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Загружаем индексы в EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Атрибут: позиция (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут: нормаль (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Атрибут: текстурные координаты (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return VAO;
}