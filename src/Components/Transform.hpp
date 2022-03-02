/*
The transform component gives an object a position and orientation in space!
*/
#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace hyd
{
    
struct TransformComponent
{
    glm::vec3 translation{0.f}; // position offset
    glm::vec3 scale{1.f};
    glm::quat orientation;

    // matrix corresponds to translate * rotate* scale transformation
    glm::mat4 mat4(){

        glm::mat4 transform = glm::toMat4(orientation);
        transform = glm::scale(transform, scale);
        transform = glm::translate(transform, translation);
        return transform;
    }
    
    glm::mat3 normalMatrix(){
        glm::vec3 inv_scale = 1.0f / scale;
        glm::mat3 normalMatrix{1.f};
        normalMatrix[0][0] = inv_scale.x;
        normalMatrix[1][1] = inv_scale.y;
        normalMatrix[2][2] = inv_scale.z;
        normalMatrix = normalMatrix*glm::toMat3(orientation);
        return normalMatrix;
    }
};

} // namespace hyd

