#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "shader.h"
#include <vector>
#include <string>

// Одна вершина
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

// Текстура меша
struct MeshTexture {
    GLuint id;
    std::string type;
    std::string path;
};

// Один меш
class Mesh {
   public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<MeshTexture> textures;

    glm::vec4 baseColorFactor{1.0f};
    bool hasBaseColorTexture{false};

    Mesh(std::vector<Vertex> v, std::vector<GLuint> i, std::vector<MeshTexture> t);
    void draw(const Shader& shader) const;
    void cleanup();

   private:
    GLuint m_vao, m_vbo, m_ebo;
    void setupMesh();
};

// Дерево
class Tree {
   public:
    Tree() = default;
    ~Tree() { cleanup(); }

    // Запрещаем копирование
    Tree(const Tree&) = delete;
    Tree& operator=(const Tree&) = delete;

    // Разрешаем перемещение
    Tree(Tree&&) = default;
    Tree& operator=(Tree&&) = default;

    bool load(const std::string& path, bool preTransform = false);

    void render(Shader& shader,
        const glm::vec3& position = glm::vec3(0.0f),
        float scale = 1.0f, float rotationY = 0.0f);

    void cleanup();

   private:
    std::vector<Mesh> m_meshes;
    std::vector<MeshTexture> m_loadedTextures;
    std::string m_directory;

    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<MeshTexture> loadMaterialTextures(
        aiMaterial* mat, aiTextureType type,
        const std::string& typeName, const aiScene* scene);
    GLuint loadTextureFromFile(const std::string& path);
    GLuint loadEmbeddedTexture(const aiTexture* tex);
    GLuint createFallbackTexture(unsigned char r, unsigned char g, unsigned char b);
};