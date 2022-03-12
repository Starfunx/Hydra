#pragma once

#include "Texture.hpp"

// std
#include <vector>
#include <memory>

namespace hyd
{
    
struct Material
{
    /* data */
    std::vector<std::shared_ptr<Texture>> m_textures;
    VkDescriptorSet m_descriptor;
};

} // namespace hyd
