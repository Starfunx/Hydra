/** 
 * This is a debug render system meant to view the 
 * depth buffers saved in a vk image
 * 
*/

#pragma once

#include "Renderer/Pipeline.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/FrameInfo.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/DescriptorSet.hpp"
#include "Renderer/SwapChain.hpp"

//libs
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>

namespace hyd
{
    
class ImageViewer
{
public:
    ImageViewer(Device& device, VkRenderPass renderPass);
    ~ImageViewer();

    ImageViewer(const ImageViewer&) = delete;
    ImageViewer &operator=(const ImageViewer&) = delete;

    void renderImage(
        FrameInfo& frameInfo,
        VkImageView imageView
        );
private:
    void createPipelineLayout(VkDescriptorSetLayout SetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<DescriptorPool> m_objectPool{};

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    std::unique_ptr<DescriptorSetLayout> m_setLayout;
    VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};

    VkSampler m_sampler;
    VkDescriptorImageInfo m_imageInfo{};

};

} // namespace se

