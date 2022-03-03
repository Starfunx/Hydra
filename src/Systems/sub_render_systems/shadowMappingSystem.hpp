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

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048
namespace hyd
{
    
class shadowMappingSystem
{
public:
    shadowMappingSystem(Device& device, VkDescriptorSetLayout globalSetLayout);
    ~shadowMappingSystem();

    shadowMappingSystem(const shadowMappingSystem&) = delete;
    shadowMappingSystem &operator=(const shadowMappingSystem&) = delete;


    void renderEntities(
        FrameInfo& frameInfo,
        entt::registry& registry);

    VkImageView getImage() {return m_shadow_map_view;}
    glm::mat4 getdepthMVP() {return m_depthMVP;}

    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout);
    void createPipeline(VkRenderPass renderPass);

    void createImage();
    void createFrameBuffer();
    void createRenderPass();

    /* data */
    Device& m_device;

    std::unique_ptr<DescriptorPool> m_objectPool{};

    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    std::unique_ptr<DescriptorSetLayout> m_materialSetLayout;
    std::vector<VkDescriptorSet> m_descriptorSets = std::vector<VkDescriptorSet>(1000);

    // shadow map  stuff
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_shadow_map_view;
    VkFramebuffer m_shadow_map_fb;
    VkRenderPass m_renderPass;
    VkCommandBuffer m_shadow_map_cmd_buf;

    VkAttachmentDescription m_attachments[2];
    VkAttachmentReference m_depth_ref;
    VkSubpassDescription m_subpass[1];
    VkRenderPassCreateInfo m_rp_info;

    std::unique_ptr<Buffer> m_uboBuffer;
    VkDescriptorSet m_globalDescriptor;

    glm::mat4 m_depthMVP{1.f};

};

} // namespace se

