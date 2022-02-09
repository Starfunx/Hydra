#pragma once

#include "Core/Window.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/DescriptorSet.hpp"

#include "sub_render_systems/simple_render_system.hpp"
#include "sub_render_systems/point_light_render_system.hpp"

// libs
#include <entt/entt.hpp>

// std
#include <memory>

namespace hyd
{
    class RenderSytstem
    {
    public:
        RenderSytstem(Device& device, Renderer& renderer);
        ~RenderSytstem();
    
        RenderSytstem (const RenderSytstem&) = delete;
        RenderSytstem& operator=(const RenderSytstem&) = delete;

        void renderEntities(float frameTime, entt::registry& registry);
    private:
        /* data */
        Device& m_device;
        Renderer& m_renderer;

        std::unique_ptr<DescriptorPool> globalPool{};

        //subrenderSystems
        std::unique_ptr<SimpleRenderSystem> m_renderSystem;
        std::unique_ptr<PointLightRenderSystem> m_pointLightRenderSystem;
        

        std::vector<VkDescriptorSet> m_globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<std::unique_ptr<Buffer>> m_uboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    };
        
} // namespace hyd
