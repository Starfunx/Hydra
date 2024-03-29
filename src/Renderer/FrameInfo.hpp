#pragma once

#include "Camera.hpp"

//lib
#include <vulkan/vulkan.h>

namespace hyd {
    struct FrameInfo
    {
        int FrameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
    };
    
} // namespace hyd