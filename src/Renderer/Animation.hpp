#pragma once

#include "Core/Types.hpp"

// libs
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.h>

// std
#include <string>
#include <vector>

namespace hyd
{

struct SQT{
    glm::vec3 scale{1.f};
    glm::quat rotation{};
    glm::vec3 translation{0.f};
};

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

struct SkeletonPose {
    std::vector<SQT> jointPoses;
};


} // namespace hyd
