/*
The transform component gives an object a position and orientation in space!
*/
#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace hyd
{
    
struct TransformComponent
{
    glm::vec3 translation; // position offset
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation;

    // matric corresponds to translate * Ry * Rz * Rx * scale transformation
    // rotation convention uses tait-bryan angles with axis order y(1), x(2), z(3)
    glm::mat4 mat4() {
        auto transform = glm::translate(glm::mat4(1.f), translation);
        transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
        transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
        transform = glm::rotate(transform, rotation.z, {0.f, 0.f, 1.f});
        transform = glm::scale(transform, scale);
        return transform;
    };
};



} // namespace hyd

