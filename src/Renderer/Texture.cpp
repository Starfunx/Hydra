#include "Texture.hpp"

#include "Ressources/Image.hpp"

// std
#include <stdexcept>

namespace hyd
{

Texture::Texture(Device& device, const std::string& filepath): m_device{device}
{
    loadTexture(filepath);
    createImageView();
    createTextureSampler();
}
    
Texture::~Texture(){
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
    vkDestroyImageView(m_device.device(), m_imageView, nullptr);

    vkDestroyImage(m_device.device(), m_image, nullptr);
    vkFreeMemory(m_device.device(), m_imageMemory, nullptr);

}
    
void Texture::loadTexture(const std::string& filepath){
    
    Image image{filepath};
    // int texWidth, texHeight, texChannels;
    // stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    // if (!pixels) {
    //     throw std::runtime_error("unable to load image: "+ filepath);
    // }

    VkDeviceSize bufferSize = image.getWidth() * image.getHeight() * 4;
    uint32_t instanceCount = 1;

    // loading in staging buffer
    Buffer stagingBuffer{
        m_device,
        bufferSize,
        instanceCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer( (void*) image.getData());

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = image.getWidth();
    imageInfo.extent.height = image.getHeight();
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


    m_device.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_image,
        m_imageMemory);

    m_device.transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_device.copyBufferToImage(
        stagingBuffer.getBuffer(), 
        m_image,
        static_cast<uint32_t>(image.getWidth()),
        static_cast<uint32_t>(image.getHeight()),
        static_cast<uint32_t>(1));
    m_device.transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void Texture::createImageView(){
    m_imageView = m_device.createImageView(m_image, VK_FORMAT_R8G8B8A8_SRGB);
}

void Texture::createTextureSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = m_device.properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkDescriptorImageInfo Texture::getImageInfo(){

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_imageView;
    imageInfo.sampler = m_sampler;
    return imageInfo;
}

} // namespace hyd
