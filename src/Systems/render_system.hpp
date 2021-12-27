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
    
class RenderSystem
{
public:
    RenderSystem(Device& device, VkRenderPass renderPass);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem &operator=(const RenderSystem&) = delete;

    void renderEntities(
        FrameInfo& frameInfo,
        entt::registry& registry);
private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);

    /* data */
    Device& m_device;

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
  
};

} // namespace se

