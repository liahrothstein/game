#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Вершина «всё-в-одном» — наш внутренний формат (межфазовый конвейер
// ассетов будет заливать именно его)
struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh {
public:
    // Собрать VAO/VBO/EBO из готовых массивов (создаётся ОДИН раз)
    bool build(const std::vector<Vertex>& verts,
               const std::vector<GLuint>& indices);
    void draw() const;                 // bind + drawElements
    void shutdown();                   // освобождение GL-ресурсов

    GLuint vao() const { return vao_; }
    size_t indexCount() const { return indexCount_; }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    size_t indexCount_ = 0;
};