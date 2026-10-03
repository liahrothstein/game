#include "render/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::proj(float aspect) const {
    return glm::ortho(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, 0.1f, 100.0f);
}

glm::mat4 Camera::view() const {
    glm::vec3 eye   (pos.x + 6.0f, 8.0f, pos.y + 6.0f);
    glm::vec3 center(pos.x, 0.0f, pos.y);
    return glm::lookAt(eye, center, glm::vec3(0, 1, 0));
}