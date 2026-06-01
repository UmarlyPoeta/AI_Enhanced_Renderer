#include "renderer/rendering/Shader.hpp"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace renderer::rendering {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);

    const unsigned int vertex = compileStage(GL_VERTEX_SHADER, vertexSource);
    const unsigned int fragment = compileStage(GL_FRAGMENT_SHADER, fragmentSource);

    programId_ = glCreateProgram();
    glAttachShader(programId_, vertex);
    glAttachShader(programId_, fragment);
    glLinkProgram(programId_);

    int success = 0;
    glGetProgramiv(programId_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programId_, 512, nullptr, infoLog);
        glDeleteProgram(programId_);
        programId_ = 0;
        throw std::runtime_error(std::string("Shader link failed: ") + infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    if (programId_ != 0) {
        glDeleteProgram(programId_);
    }
}

Shader::Shader(Shader&& other) noexcept : programId_(other.programId_) {
    other.programId_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (programId_ != 0) {
            glDeleteProgram(programId_);
        }
        programId_ = other.programId_;
        other.programId_ = 0;
    }
    return *this;
}

void Shader::use() const {
    glUseProgram(programId_);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(programId_, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programId_, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programId_, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(programId_, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(programId_, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(programId_, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

unsigned int Shader::compileStage(unsigned int type, const std::string& source) {
    const unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compile failed: ") + infoLog);
    }
    return shader;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace renderer::rendering
