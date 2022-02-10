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
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
  
};

} // namespace se

