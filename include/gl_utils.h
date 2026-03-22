#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

// Структура вершины (позиция + нормаль + текстурные координаты)
struct Vertex {
    float x, y, z;      // позиция
    float nx, ny, nz;   // нормаль
    float u, v;         // текстурные координаты
};

// Проверка ошибок OpenGL (для отладки)
void checkGLError(const std::string& location);

// Компиляция шейдера
GLuint compileShader(GLenum type, const std::string& source);

// Создание шейдерной программы из файлов
GLuint createProgram(const std::string& vertexPath, const std::string& fragmentPath);

// Создание VAO из вершин и индексов
GLuint createVAO(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);