#include "Terrain.h"
#include "Tree.h"
#include "Game.h"
#include <cmath>
#include <vector>
#include <iostream>
#include "stb_image.h"
#include <glm/gtc/noise.hpp>
#include <glm/common.hpp>
#include <algorithm>

Terrain::Terrain() : m_vao(0), m_vbo(0), m_texture(0), m_vertexCount(0) {}

Terrain::~Terrain() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_texture) glDeleteTextures(1, &m_texture);
}

// Генерация карты с помощью шума Перлина
float Terrain::getHeight(float x, float z) const{
    
    glm::vec2 pos = glm::vec2(x, z) * 0.1f;

    float baseHeight = glm::perlin(pos) * 2.0f;

    // Сглаживаем края карты для забора
    const float MAP_LIMIT = 50.0f;
    const float EDGE_BLEND = 5.0f;

    float distToEdgeX = MAP_LIMIT - std::abs(x);
    float distToEdgeZ = MAP_LIMIT - std::abs(z);
    float minDistToEdge = (glm::min)(distToEdgeX, distToEdgeZ);

  
    float flattenFactor = glm::clamp(minDistToEdge / EDGE_BLEND, 0.0f, 1.0f);
    flattenFactor = flattenFactor * flattenFactor * (3.0f - 2.0f * flattenFactor);

    return baseHeight * flattenFactor;
}

glm::vec3 Terrain::getNormal(float x, float z) const {
    float eps = 0.1f;
    float hL = getHeight(x - eps, z);
    float hR = getHeight(x + eps, z);
    float hD = getHeight(x, z - eps);
    float hU = getHeight(x, z + eps);
    return glm::normalize(glm::vec3(hL - hR, 2.0f * eps, hD - hU));
}


// Натягиваем текстуру на землю
void Terrain::loadTexture(const char* path) {
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Настройки повторения текстуры 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
 
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

void Terrain::init(const char* texturePath) {
    std::vector<float> verts;
    const int gridSize = 100;
    const float step = 1.0f;
    const float offset = (gridSize * step) / 2.0f;

    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            float x0 = x * step - offset, z0 = z * step - offset;
            float x1 = (x + 1) * step - offset, z1 = (z + 1) * step - offset;

            auto addV = [&](float _x, float _z) {
                glm::vec3 p(_x, getHeight(_x, _z), _z);
                glm::vec3 n = getNormal(_x, _z);
                verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z);
                verts.push_back(n.x); verts.push_back(n.y); verts.push_back(n.z);
                verts.push_back(_x * 0.2f); verts.push_back(_z * 0.2f);
                };

            addV(x0, z0); addV(x1, z0); addV(x1, z1);
            addV(x0, z0); addV(x1, z1); addV(x0, z1);
        }
    }
    m_vertexCount = static_cast<int>(verts.size() / 8);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    loadTexture(texturePath);
}

void Terrain::render(const Shader& shader) {
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setMat3("normalMatrix", glm::mat3(1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
}