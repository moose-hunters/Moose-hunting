#include "Cube.h"
#include <iostream>

Cube::Cube() {
    // Вершины: позиция (x,y,z) + цвет (r,g,b)
    vertices = {
        // Передняя грань (z = 0.5)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 0: красный
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // 1: зеленый
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 2: синий
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // 3: желтый

        // Задняя грань (z = -0.5)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // 4: пурпурный
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // 5: голубой
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  // 6: белый
        -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f   // 7: серый
    };

    indices = {
        // Передняя грань
        0, 1, 2,  0, 2, 3,
        // Задняя грань
        4, 6, 5,  4, 7, 6,
        // Левая грань
        4, 3, 7,  4, 0, 3,
        // Правая грань
        1, 5, 2,  5, 6, 2,
        // Верхняя грань
        3, 2, 6,  3, 6, 7,
        // Нижняя грань
        4, 5, 1,  4, 1, 0
    };

    setupBuffers();
}

Cube::~Cube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Cube::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        indices.data(), GL_STATIC_DRAW);

    // Атрибут 0: позиция (3 компонента)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут 1: цвет (3 компонента)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cube::render() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}