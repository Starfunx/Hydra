#pragma once

#include "Core/Types.hpp"

// libs
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

// std
#include <string>
#include <vector>

namespace hyd
{


struct SkeletonJoint {
    std::string name;
    i16 parentIndex;
    glm::mat4 invBindPose;

};

struct Skeleton {
    i16 jointCount;
    std::vector<SkeletonJoint> joints;
    VkDescriptorSet descriptorSet;
};


} // namespace hyd
