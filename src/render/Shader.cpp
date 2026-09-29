#include "Shader.h"
#include <iostream>

static GLuint compile(GLenum type, const std::string& src) {
    GLuint sh = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(sh, 1, &c, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n" << src << "\n";
    }
    return sh;
}

void Shader::build(const std::string& vertSrc, const std::string& fragSrc) {
    GLuint v = compile(GL_VERTEX_SHADER, vertSrc);
    GLuint f = compile(GL_FRAGMENT_SHADER, fragSrc);
    id = glCreateProgram();
    glAttachShader(id, v);
    glAttachShader(id, f);
    glLinkProgram(id);
    GLint ok = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id, 1024, nullptr, log);
        std::cerr << "Program link error:\n" << log << "\n";
    }
    glDeleteShader(v);
    glDeleteShader(f);
}

void Shader::use() const { glUseProgram(id); }

void Shader::setMat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, &m[0][0]);
}

void Shader::setVec2(const char* name, const glm::vec2& v) const {
    glUniform2fv(glGetUniformLocation(id, name), 1, &v[0]);
}