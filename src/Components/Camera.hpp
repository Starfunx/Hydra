#pragma once

#include "Renderer/Camera.hpp"

// libs
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace hyd
{
    
struct CameraComponent
{
    /* data */
    glm::vec3 position{0.f};
    glm::quat orientation;

    Camera camera{};
};


} // namespace hyd
