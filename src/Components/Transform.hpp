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
    
struct Transform2dComponent
{
    glm::vec2 translation; // position offset
    glm::vec2 scale; // position offset
    float rotation; // position offset

    glm::mat2 mat2() {
        const float c = glm::cos(rotation);
        const float s = glm::sin(rotation);
        glm::mat2 rotationMatrix {{c, s}, {-s, c}};


        glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
        return rotationMatrix*scaleMat;
        };
};


} // namespace hyd

