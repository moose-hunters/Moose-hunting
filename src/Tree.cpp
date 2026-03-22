#include "Tree.h"
#include "gl_utils.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

Tree::Tree()
    : m_VAO(0)
    , m_VBO(0)
    , m_EBO(0)
    , m_indexCount(0)
{
}

Tree::~Tree() {
    cleanup();
}

bool Tree::load(const std::string& glbPath) {
    // Получаем путь к папке с моделью для поиска текстур
    size_t lastSlash = glbPath.find_last_of("/\\");
    std::string basePath = (lastSlash != std::string::npos) ? glbPath.substr(0, lastSlash + 1) : "";

    // Настраиваем загрузчик
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    // Настраиваем callback для загрузки текстур
    tinygltf::FsCallbacks fsCallbacks = {};
    fsCallbacks.FileExists = [](const std::string&, void*) -> bool { return true; };
    fsCallbacks.ExpandFilePath = [](const std::string& path, void*) -> std::string { return path; };
    fsCallbacks.ReadWholeFile = [basePath](std::vector<unsigned char>* data, std::string* err,
        const std::string& filepath, void*) -> bool {
            std::string fullPath = basePath + filepath;
            FILE* fp = fopen(fullPath.c_str(), "rb");
            if (!fp) {
                if (err) *err = "Cannot open file: " + fullPath;
                return false;
            }
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            data->resize(size);
            fread(data->data(), 1, size, fp);
            fclose(fp);
            return true;
        };
    loader.SetFsCallbacks(fsCallbacks);

    // Загружаем GLB файл
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, glbPath);
    if (!ret) {
        std::cerr << "Failed to load model: " << glbPath << std::endl;
        return false;
    }

    // Загружаем текстуры
    m_textures.clear();
    for (const auto& image : model.images) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width = image.width;
        int height = image.height;
        int channels = image.component;
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        Texture tex;
        tex.id = textureID;
        tex.type = "diffuse";
        m_textures.push_back(tex);
    }

    // Собираем геометрию из всех мешей
    std::vector<Vertex> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int vertexOffset = 0;

    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            // Получаем позиции
            auto posIt = prim.attributes.find("POSITION");
            if (posIt == prim.attributes.end()) continue;

            const tinygltf::Accessor& posAccessor = model.accessors[posIt->second];
            const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];
            const float* positions = reinterpret_cast<const float*>(
                &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);
            int vertexCount = posAccessor.count;

            // Получаем нормали
            auto normIt = prim.attributes.find("NORMAL");
            const float* normals = nullptr;
            if (normIt != prim.attributes.end()) {
                const tinygltf::Accessor& normAccessor = model.accessors[normIt->second];
                const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
                const tinygltf::Buffer& normBuffer = model.buffers[normView.buffer];
                normals = reinterpret_cast<const float*>(
                    &normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
            }

            // Получаем UV
            auto uvIt = prim.attributes.find("TEXCOORD_0");
            const float* uvs = nullptr;
            bool hasUV = (uvIt != prim.attributes.end());
            if (hasUV) {
                const tinygltf::Accessor& uvAccessor = model.accessors[uvIt->second];
                const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
                const tinygltf::Buffer& uvBuffer = model.buffers[uvView.buffer];
                uvs = reinterpret_cast<const float*>(
                    &uvBuffer.data[uvView.byteOffset + uvAccessor.byteOffset]);
            }

            // Добавляем вершины
            for (int i = 0; i < vertexCount; i++) {
                Vertex v;
                v.x = positions[i * 3];
                v.y = positions[i * 3 + 1];
                v.z = positions[i * 3 + 2];

                if (normals) {
                    v.nx = normals[i * 3];
                    v.ny = normals[i * 3 + 1];
                    v.nz = normals[i * 3 + 2];
                }
                else {
                    v.nx = v.ny = v.nz = 0;
                }

                if (hasUV && uvs) {
                    v.u = uvs[i * 2];
                    v.v = uvs[i * 2 + 1];
                }
                else {
                    v.u = v.v = 0.0f;
                }

                allVertices.push_back(v);
            }

            // Получаем индексы
            const tinygltf::Accessor& idxAccessor = model.accessors[prim.indices];
            const tinygltf::BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
            const tinygltf::Buffer& idxBuffer = model.buffers[idxView.buffer];
            int indexCount = idxAccessor.count;

            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const unsigned int* data = reinterpret_cast<const unsigned int*>(
                    &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);
                for (int i = 0; i < indexCount; i++) {
                    allIndices.push_back(vertexOffset + data[i]);
                }
            }
            else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const unsigned short* data = reinterpret_cast<const unsigned short*>(
                    &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);
                for (int i = 0; i < indexCount; i++) {
                    allIndices.push_back(vertexOffset + data[i]);
                }
            }

            vertexOffset += vertexCount;
        }
    }

    if (allVertices.empty() || allIndices.empty()) {
        std::cerr << "Error: No geometry data to render!" << std::endl;
        return false;
    }

    m_indexCount = allIndices.size();
    setupMesh(allVertices, allIndices);

    return true;
}

void Tree::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    m_VAO = createVAO(vertices, indices);
    m_indexCount = indices.size();
}

void Tree::render(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& position, float scale) {
    if (m_VAO == 0) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram == 0) return;

    GLint modelLoc = glGetUniformLocation(currentProgram, "model");
    GLint viewLoc = glGetUniformLocation(currentProgram, "view");
    GLint projLoc = glGetUniformLocation(currentProgram, "projection");

    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    if (!m_textures.empty()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures[0].id);
        GLint texLoc = glGetUniformLocation(currentProgram, "texture_diffuse");
        if (texLoc != -1) glUniform1i(texLoc, 0);
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Tree::cleanup() {
    for (auto& tex : m_textures) {
        if (tex.id) glDeleteTextures(1, &tex.id);
    }
    m_textures.clear();

    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    m_indexCount = 0;
}