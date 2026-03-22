#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader();
    ~Shader();
    
    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use();
    
    void setMat4(const std::string& name, const glm::mat4& mat);
    void setVec3(const std::string& name, const glm::vec3& vec);
    void setFloat(const std::string& name, float value);
    
    GLuint getID() const { return m_program; }
    
private:
    GLuint m_program;
};