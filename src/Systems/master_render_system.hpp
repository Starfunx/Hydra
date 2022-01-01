#pragma once

#include "Core/Window.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/DescriptorSet.hpp"

#include "render_system.hpp"
#include "point_light_render_system.hpp"

// libs
#include <entt/entt.hpp>

// std
#include <memory>

namespace hyd
{
    class MasterRenderSytstem
    {
    public:
        MasterRenderSytstem(Device& device, Renderer& renderer);
        ~MasterRenderSytstem();
    
        MasterRenderSytstem (const MasterRenderSytstem&) = delete;
        MasterRenderSytstem& operator=(const MasterRenderSytstem&) = delete;

        void renderEntities(float frameTime, entt::registry& registry);
    private:
        /* data */
        Device& m_device;
        Renderer& m_renderer;

        std::unique_ptr<DescriptorPool> globalPool{};

        //subrenderSystems
        std::unique_ptr<RenderSystem> m_renderSystem;
        std::unique_ptr<PointLightRenderSystem> m_pointLightRenderSystem;
        

        std::vector<VkDescriptorSet> m_globalDescriptorSets;
        std::vector<std::unique_ptr<Buffer>> m_uboBuffers;
    };
        
} // namespace hyd
