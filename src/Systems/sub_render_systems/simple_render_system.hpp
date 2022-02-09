#pragma once

#include "Renderer/Pipeline.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/FrameInfo.hpp"

//libs
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>

namespace hyd
{
    
class SimpleRenderSystem
{
public:
    SimpleRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem&) = delete;

    void renderEntities(
        FrameInfo& frameInfo,
        entt::registry& registry);
private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
  
};

} // namespace se

