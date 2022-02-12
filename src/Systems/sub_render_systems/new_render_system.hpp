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
    
class NewRenderSystem
{
public:
    NewRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~NewRenderSystem();

    NewRenderSystem(const NewRenderSystem&) = delete;
    NewRenderSystem &operator=(const NewRenderSystem&) = delete;

    void renderEntities(
        FrameInfo& frameInfo,
        entt::registry& registry);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<DescriptorPool> m_objectPool{};

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    std::vector<VkDescriptorSet> m_descriptorSets = std::vector<VkDescriptorSet>(1000);
    std::vector<std::unique_ptr<Buffer>> m_uboBuffers = std::vector<std::unique_ptr<Buffer>>(1000);

};

} // namespace se

