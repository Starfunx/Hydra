#pragma once

#include "Renderer/DescriptorSet.hpp"
#include "Renderer/Texture.hpp"

namespace hyd
{
    
    struct Material
    {
    /* data */
    std::string name;
    VkDescriptorSet material_descriptor{VK_NULL_HANDLE};
    std::vector<std::shared_ptr<Texture>> textures;
    };


} // namespace hyd
