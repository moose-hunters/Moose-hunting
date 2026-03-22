#ifndef TREE_H
#define TREE_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex;

typedef unsigned int GLuint;

struct Texture {
    GLuint id;
    std::string type;
};

class Tree {
public:
    Tree();
    ~Tree();

    bool load(const std::string& glbPath);
    void render(const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& position, float scale = 1.0f);
    void cleanup();

private:
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;
    int m_indexCount;
    std::vector<Texture> m_textures;

    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};

#endif