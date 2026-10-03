#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    glm::vec2 pos{0.0f, 0.0f};      // позиция на «земле» (метры)

    glm::mat4 proj(float aspect) const;   // aspect передаём снаружи
    glm::mat4 view() const;
};