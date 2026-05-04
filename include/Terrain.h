#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

class Terrain {
public:
    Terrain();
    ~Terrain();

    void init(const char* texturePath);
    void render(const Shader& shader);

    float getHeight(float x, float z) const;
    glm::vec3 getNormal(float x, float z) const;

private:
    GLuint m_vao, m_vbo, m_texture;
    int m_vertexCount;

    void loadTexture(const char* path);
};