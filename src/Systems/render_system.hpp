#pragma once

#include "Core/Window.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/DescriptorSet.hpp"

#include "sub_render_systems/point_light_render_system.hpp"
#include "sub_render_systems/object_render_system.hpp"
#include "sub_render_systems/skybox_render_system.hpp"
#include "sub_render_systems/shadowMappingSystem.hpp"
#include "sub_render_systems/imageViewer.hpp"

// libs
#include <entt/entt.hpp>

// std
#include <memory>

namespace hyd
{
    class RenderSystem
    {
    public:
        RenderSystem(Device& device, Renderer& renderer);
        ~RenderSystem();
    
        RenderSystem (const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        void renderEntities(float frameTime, entt::registry& registry);
    private:
        /* data */
        Device& m_device;
        Renderer& m_renderer;

        // global pool, for objects shared by all renderers
        std::unique_ptr<DescriptorPool> globalPool{};

        //subrenderSystems
        std::unique_ptr<SkyboxRenderSystem> m_skyboxRenderSystem;
        std::unique_ptr<PointLightRenderSystem> m_pointLightRenderSystem;
        std::unique_ptr<ObjectRenderSystem> m_objectRenderSystem;
        std::unique_ptr<shadowMappingSystem> m_shadow_mapping_system;
        std::unique_ptr<ImageViewer> m_imageViewer;
        

        std::vector<VkDescriptorSet> m_globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<std::unique_ptr<Buffer>> m_uboBuffers = std::vector<std::unique_ptr<Buffer>>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    
        VkSampler m_sampler;
    };
        
} // namespace hyd
