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
    
class ObjectRenderSystem
{
public:
    ObjectRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkImageView imageView);
    ~ObjectRenderSystem();

    ObjectRenderSystem(const ObjectRenderSystem&) = delete;
    ObjectRenderSystem &operator=(const ObjectRenderSystem&) = delete;

    void renderEntities(
        FrameInfo& frameInfo,
        entt::registry& registry,
        const glm::mat4& lightDepthMVP,
        VkImageView imageView,
        float aspectRatio);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<DescriptorPool> m_globalPool{};
    std::unique_ptr<DescriptorPool> m_objectPool{};

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;


    std::vector<VkDescriptorSet> m_descriptorSets = std::vector<VkDescriptorSet>(1000);
    std::vector<VkDescriptorSet> m_globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);

    std::vector<std::unique_ptr<Buffer>> m_uboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
   
    std::vector<VkDescriptorImageInfo> m_descriptorImageInfo = std::vector<VkDescriptorImageInfo>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSampler> m_sampler = std::vector<VkSampler>(SwapChain::MAX_FRAMES_IN_FLIGHT);

};

} // namespace se

