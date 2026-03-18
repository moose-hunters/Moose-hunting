#pragma once

#include <glad/glad.h>
#include <vector>

class Cube {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void setupBuffers();

public:
    Cube();
    ~Cube();

    void render();
};