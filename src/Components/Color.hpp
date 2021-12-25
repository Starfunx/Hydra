/*
The transform component gives an object a position and orientation in space!
*/
#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace hyd
{
    
struct ColorComponent
{
    glm::vec3 m_color {};
};


} // namespace hyd

