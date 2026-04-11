#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() {
    if (m_id) glDeleteProgram(m_id);
}

std::string Shader::readFile(const std::string& path) const {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Shader: cannot open file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compileShader(GLenum type, const std::string& src) const {
    GLuint shader = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error ("
                  << (type == GL_VERTEX_SHADER ? "VERT" : "FRAG")
                  << "):\n"
                  << log << std::endl;
    }
    return shader;
}

bool Shader::load(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    m_id = glCreateProgram();
    glAttachShader(m_id, vert);
    glAttachShader(m_id, frag);
    glLinkProgram(m_id);

    if (!checkLink(m_id)) return false;

    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
}

bool Shader::checkLink(GLuint program) const {
    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error:\n"
                  << log << std::endl;
        return false;
    }
    return true;
}

void Shader::use() const { glUseProgram(m_id); }

void Shader::setBool(const std::string& n, bool v) const { glUniform1i(glGetUniformLocation(m_id, n.c_str()), (int)v); }
void Shader::setInt(const std::string& n, int v) const { glUniform1i(glGetUniformLocation(m_id, n.c_str()), v); }
void Shader::setFloat(const std::string& n, float v) const { glUniform1f(glGetUniformLocation(m_id, n.c_str()), v); }
void Shader::setVec2(const std::string& n, const glm::vec2& v) const { glUniform2fv(glGetUniformLocation(m_id, n.c_str()), 1, glm::value_ptr(v)); }
void Shader::setVec3(const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(m_id, n.c_str()), 1, glm::value_ptr(v)); }
void Shader::setVec4(const std::string& n, const glm::vec4& v) const { glUniform4fv(glGetUniformLocation(m_id, n.c_str()), 1, glm::value_ptr(v)); }
void Shader::setMat3(const std::string& n, const glm::mat3& m) const { glUniformMatrix3fv(glGetUniformLocation(m_id, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::setMat4(const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(m_id, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }