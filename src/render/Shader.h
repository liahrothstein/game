#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint id = 0;

    void build(const std::string& vertSrc, const std::string& fragSrc);
    void use() const;
    void setMat4(const char* name, const glm::mat4& m) const;
    void setVec2(const char* name, const glm::vec2& v) const;
};