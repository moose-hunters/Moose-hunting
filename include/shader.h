#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
   public:
    Shader() : m_id(0) {}
    ~Shader();

    bool load(const std::string& vertPath, const std::string& fragPath);
    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& v) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;
    void setVec4(const std::string& name, const glm::vec4& v) const;
    void setMat3(const std::string& name, const glm::mat3& m) const;
    void setMat4(const std::string& name, const glm::mat4& m) const;

    GLuint id() const { return m_id; }

   private:
    GLuint m_id;

    std::string readFile(const std::string& path) const;
    GLuint compileShader(GLenum type, const std::string& src) const;
    bool checkLink(GLuint program) const;
};