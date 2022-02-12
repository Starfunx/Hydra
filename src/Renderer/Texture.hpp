#pragma once

#include "Device.hpp"
#include "Buffer.hpp"

//libs
#include <vulkan/vulkan.h>

// std
#include <string>
#include <memory>

namespace hyd
{
class Texture
{
public:
    Texture(Device& device, const std::string& filepath);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture &operator=(const Texture&) = delete;

    VkDescriptorImageInfo getImageInfo();

private:
    void loadTexture(const std::string& filepath);
    void createImageView();
    void createTextureSampler();
    
    /* data */

    Device& m_device;

    VkImage m_image; 
    VkDeviceMemory m_imageMemory;
    VkImageView m_imageView;

    VkSampler m_sampler;
};

} // namespace hyd
