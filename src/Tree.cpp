#include "Tree.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ==================== Mesh ====================

Mesh::Mesh(std::vector<Vertex> v, std::vector<GLuint> i, std::vector<MeshTexture> t)
    : vertices(std::move(v)), indices(std::move(i)), textures(std::move(t)) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
        indices.data(), GL_STATIC_DRAW);

    // layout 0 — position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, position));
    // layout 1 — normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal));
    // layout 2 — texCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoords));
    // layout 3 — tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, tangent));
    // layout 4 — bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}

void Mesh::draw(const Shader& shader) const {
    unsigned int diffuseN = 0;
    unsigned int specularN = 0;

    // Привязываем текстуры
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string name;
        if (textures[i].type == "texture_diffuse") {
            name = "texture_diffuse" + std::to_string(diffuseN++);
        } else if (textures[i].type == "texture_specular") {
            name = "texture_specular" + std::to_string(specularN++);
        } else {
            name = textures[i].type + std::to_string(i);
        }
        shader.setInt(name, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // Передаём базовый цвет (для мешей без текстуры)
    shader.setVec4("baseColorFactor", baseColorFactor);
    shader.setBool("hasTexture", !textures.empty() && hasBaseColorTexture);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

void Mesh::cleanup() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

// ==================== Tree ====================

bool Tree::load(const std::string& path) {
    Assimp::Importer importer;

    // aiProcess_Triangulate — гарантируем треугольники
    // aiProcess_FlipUVs     — переворачиваем UV по Y (OpenGL vs DirectX)
    // aiProcess_CalcTangentSpace — считаем тангенты
    // aiProcess_GenNormals  — генерируем нормали если их нет
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_GenSmoothNormals |
            aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Директория нужна для загрузки внешних текстур
    m_directory = path.substr(0, path.find_last_of("/\\"));
    std::cout << "Model directory: " << m_directory << std::endl;
    std::cout << "Meshes: " << scene->mNumMeshes
              << "  Materials: " << scene->mNumMaterials
              << "  Textures embedded: " << scene->mNumTextures << std::endl;

    processNode(scene->mRootNode, scene);
    return true;
}

void Tree::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Tree::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<MeshTexture> textures;

    // ---- Вершины ----
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v{};
        v.position = {mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z};

        if (mesh->HasNormals()) {
            v.normal = {mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z};
        }

        if (mesh->mTextureCoords[0]) {
            v.texCoords = {mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y};
        }

        if (mesh->HasTangentsAndBitangents()) {
            v.tangent = {mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z};
            v.bitangent = {mesh->mBitangents[i].x,
                mesh->mBitangents[i].y,
                mesh->mBitangents[i].z};
        }
        vertices.push_back(v);
    }

    // ---- Индексы ----
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // ---- Материалы / Текстуры ----
    glm::vec4 baseColor{1.0f};
    bool hasBaseColorTex = false;

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        // GLTF PBR albedo
        aiColor4D color;
        if (AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
            baseColor = {color.r, color.g, color.b, color.a};
        }
        // Для GLTF2 — ключ base color
        if (AI_SUCCESS == mat->Get(AI_MATKEY_BASE_COLOR, color)) {
            baseColor = {color.r, color.g, color.b, color.a};
        }

        // Диффузные текстуры (GLTF хранит их как aiTextureType_DIFFUSE или BASE_COLOR)
        auto diff = loadMaterialTextures(mat, aiTextureType_DIFFUSE,
            "texture_diffuse", scene);
        textures.insert(textures.end(), diff.begin(), diff.end());

        // GLTF base color texture
        auto base = loadMaterialTextures(mat, aiTextureType_BASE_COLOR,
            "texture_diffuse", scene);
        textures.insert(textures.end(), base.begin(), base.end());

        hasBaseColorTex = !textures.empty();

        // Specular
        auto spec = loadMaterialTextures(mat, aiTextureType_SPECULAR,
            "texture_specular", scene);
        textures.insert(textures.end(), spec.begin(), spec.end());
    }

    // Если текстур вообще нет — добавляем белую заглушку
    if (!hasBaseColorTex) {
        MeshTexture fallback;
        fallback.id = createFallbackTexture(
            (unsigned char)(baseColor.r * 255),
            (unsigned char)(baseColor.g * 255),
            (unsigned char)(baseColor.b * 255));
        fallback.type = "texture_diffuse";
        fallback.path = "__fallback__";
        textures.push_back(fallback);
    }

    Mesh result(vertices, indices, textures);
    result.baseColorFactor = baseColor;
    result.hasBaseColorTexture = hasBaseColorTex;
    return result;
}

std::vector<MeshTexture> Tree::loadMaterialTextures(
    aiMaterial* mat, aiTextureType type,
    const std::string& typeName, const aiScene* scene) {
    std::vector<MeshTexture> result;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texPath = str.C_Str();

        // Проверяем — не грузили ли уже эту текстуру
        bool already = false;
        for (auto& lt : m_loadedTextures) {
            if (lt.path == texPath) {
                result.push_back(lt);
                already = true;
                break;
            }
        }
        if (already) continue;

        MeshTexture tex;
        tex.type = typeName;
        tex.path = texPath;

        // Встроенная текстура GLB (путь начинается с '*')
        if (texPath[0] == '*') {
            int idx = std::stoi(texPath.substr(1));
            tex.id = loadEmbeddedTexture(scene->mTextures[idx]);
        } else {
            // Внешний файл
            tex.id = loadTextureFromFile(m_directory + "/" + texPath);
        }

        m_loadedTextures.push_back(tex);
        result.push_back(tex);
    }
    return result;
}

GLuint Tree::loadEmbeddedTexture(const aiTexture* tex) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int w, h, ch;
    unsigned char* data = nullptr;

    if (tex->mHeight == 0) {
        // Сжатые данные (PNG/JPG внутри GLB)
        data = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(tex->pcData),
            tex->mWidth, &w, &h, &ch, 0);
    } else {
        // Несжатые RGBA8888
        w = tex->mWidth;
        h = tex->mHeight;
        ch = 4;
        data = reinterpret_cast<unsigned char*>(tex->pcData);
    }

    if (data) {
        GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB
                                                     : GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        if (tex->mHeight == 0) stbi_image_free(data);
    } else {
        std::cerr << "Embedded texture decode failed: "
                  << stbi_failure_reason() << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint Tree::loadTextureFromFile(const std::string& path) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (data) {
        GLenum fmt = (ch == 4) ? GL_RGBA : (ch == 3) ? GL_RGB
                                                     : GL_RED;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

GLuint Tree::createFallbackTexture(unsigned char r, unsigned char g, unsigned char b) {
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    unsigned char px[3] = {r, g, b};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

void Tree::render(Shader& shader,
    const glm::vec3& position,
    float scale) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);

    // Нормальная матрица для корректного освещения при масштабировании
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    shader.setMat3("normalMatrix", normalMatrix);

    for (auto& mesh : m_meshes)
        mesh.draw(shader);
}

void Tree::cleanup() {
    for (auto& m : m_meshes) m.cleanup();
    m_meshes.clear();

    // Удаляем загруженные текстуры
    for (auto& t : m_loadedTextures)
        glDeleteTextures(1, &t.id);
    m_loadedTextures.clear();
}