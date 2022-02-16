#pragma once

#include "Renderer/Pipeline.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/FrameInfo.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/DescriptorSet.hpp"
#include "Renderer/SwapChain.hpp"
#include "Renderer/Model.hpp"

//libs
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>

namespace hyd
{
    
class SkyboxRenderSystem
{
public:
    SkyboxRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SkyboxRenderSystem();

    SkyboxRenderSystem(const SkyboxRenderSystem&) = delete;
    SkyboxRenderSystem &operator=(const SkyboxRenderSystem&) = delete;

    void render(FrameInfo& frameInfo);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<DescriptorPool> m_objectPool{};

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;
    VkDescriptorSet m_skyboxDescriptor;

    std::unique_ptr<Model> m_skybox_model;


    //
    VkImage m_image; 
    VkDeviceMemory m_imageMemory;
    VkImageView m_imageView;
    VkSampler m_sampler;

    void loadImage();

};

} // namespace se

