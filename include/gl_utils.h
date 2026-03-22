#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

// стуктура для вершины
struct Vertex {
    float x, y, z;      // позиция
    float nx, ny, nz;   // нормаль
    float u, v;         // текстурные координаты
};

// проверяем ошибки для отладки
void checkGLError(const std::string& location);

// шейдеры
GLuint compileShader(GLenum type, const std::string& source);

// создание шейдерной программы из файлов
GLuint createProgram(const std::string& vertexPath, const std::string& fragmentPath);

// создание VAO из вершин и индексов
GLuint createVAO(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);