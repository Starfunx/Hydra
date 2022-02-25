#include "shadowMappingSystem.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Components/Material.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <cassert>
#include <array>
#include <iostream>

namespace hyd
{

struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f}; // projection * view * model
    glm::mat4 normalMatrix{1.f};
};


shadowMappingSystem::shadowMappingSystem(Device& device, VkDescriptorSetLayout globalSetLayout):
m_device{device}{

    m_objectPool = 
    DescriptorPool::Builder(m_device)
        .setMaxSets(1000) // large amount alocated
        // .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .build();

    // allocate descriptor sets from the descriptor pool
    m_materialSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    createImage();
    createRenderPass();
    createFrameBuffer();
    createPipelineLayout(globalSetLayout, m_materialSetLayout->getDescriptorSetLayout());
    createPipeline(m_renderPass);
}

shadowMappingSystem::~shadowMappingSystem(){
    vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);
    vkDestroyImage(m_device.device(), m_image, nullptr);
    vkFreeMemory(m_device.device(), m_memory, nullptr);
    vkDestroyImageView(m_device.device(), m_shadow_map_view, nullptr);
    vkDestroyFramebuffer(m_device.device(), m_shadow_map_fb, nullptr);
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void shadowMappingSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout) {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, objectSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to create pipeline layout");
    }

}

void shadowMappingSystem::createPipeline(VkRenderPass renderPass){
    assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);

    
    pipelineConfig.colorBlendInfo.attachmentCount = 0;
    pipelineConfig.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        "../shaders/shadow_mapping.vert.spv",
        pipelineConfig);
}


void shadowMappingSystem::renderEntities(
     FrameInfo& frameInfo,
     entt::registry& registry){
    auto view = registry.view<TransformComponent, MeshComponent, Material>();


    // bind pipline
    m_pipeline->bind(m_shadow_map_cmd_buf);

    // bind global descriptor set - at set #0
    vkCmdBindDescriptorSets(
            m_shadow_map_cmd_buf,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

    static int material_index{0};
    // for each object/Materials
    for(auto entity: view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &mesh = view.get<MeshComponent>(entity);
        auto &material= view.get<Material>(entity);

        if (material.material_descriptor == VK_NULL_HANDLE){
            // write a descriptor in the pool
            auto imageInfo = material.textures[0]->getImageInfo();
            DescriptorWriter(*m_materialSetLayout, *m_objectPool)
                .writeImage(0, &imageInfo)
                .build(m_descriptorSets[material_index]);
            material.material_descriptor = m_descriptorSets[material_index];
            material_index++;
        }

        // TODO reduce binds calls by grouping objects by materials
        // bind material descriptor set - at set #1
        vkCmdBindDescriptorSets(
                m_shadow_map_cmd_buf,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                1,
                1,
                &material.material_descriptor,
                0,
                nullptr);
    
        // for each obj that uses the material
            // bind object descriptor set - at set #2

            SimplePushConstantData push{};
            push.modelMatrix = transform.mat4();
            push.normalMatrix = transform.normalMatrix();

            vkCmdPushConstants(
                m_shadow_map_cmd_buf,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            // bind obj model
            mesh.model->bind(m_shadow_map_cmd_buf);
            // draw object
            mesh.model->draw(m_shadow_map_cmd_buf);
    }
}



void shadowMappingSystem::createImage(){
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_D32_SFLOAT;
    image_info.extent.width = SHADOW_MAP_WIDTH;
    image_info.extent.height = SHADOW_MAP_HEIGHT;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = NULL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.flags = 0;
    
    // vkCreateImage(m_device.device(), &image_info, NULL, &m_image);
    m_device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);



    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = m_image;
    view_info.format = VK_FORMAT_D32_SFLOAT;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.flags = 0;

    vkCreateImageView(m_device.device(), &view_info, NULL, &m_shadow_map_view);
}

void shadowMappingSystem::createRenderPass()
{
 
   // Depth attachment (shadow map)
   m_attachments[0].format = VK_FORMAT_D32_SFLOAT;
   m_attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
   m_attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   m_attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   m_attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   m_attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   m_attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   m_attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   m_attachments[0].flags = 0;
 
   // Attachment references from subpasses
   m_depth_ref.attachment = 0;
   m_depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
 
   // Subpass 0: shadow map rendering
   m_subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   m_subpass[0].flags = 0;
   m_subpass[0].inputAttachmentCount = 0;
   m_subpass[0].pInputAttachments = NULL;
   m_subpass[0].colorAttachmentCount = 0;
   m_subpass[0].pColorAttachments = NULL;
   m_subpass[0].pResolveAttachments = NULL;
   m_subpass[0].pDepthStencilAttachment = &m_depth_ref;
   m_subpass[0].preserveAttachmentCount = 0;
   m_subpass[0].pPreserveAttachments = NULL;
 
   // Create render pass
   m_rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   m_rp_info.pNext = NULL;
   m_rp_info.attachmentCount = 1;
   m_rp_info.pAttachments = m_attachments;
   m_rp_info.subpassCount = 1;
   m_rp_info.pSubpasses = m_subpass;
   m_rp_info.dependencyCount = 0;
   m_rp_info.pDependencies = NULL;
   m_rp_info.flags = 0;
 
   vkCreateRenderPass(m_device.device(), &m_rp_info, NULL, &m_renderPass);
 }

void shadowMappingSystem::createFrameBuffer()
{
    VkFramebufferCreateInfo fb_info;
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = m_renderPass;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &m_shadow_map_view;
    fb_info.width = SHADOW_MAP_WIDTH;
    fb_info.height = SHADOW_MAP_HEIGHT;
    fb_info.layers = 1;
    fb_info.flags = 0;
 
    vkCreateFramebuffer(m_device.device(), &fb_info, NULL, &m_shadow_map_fb);
}


void shadowMappingSystem::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
    // assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if freame is not in progress!");
    // assert(commandBuffer == getCurrentCommandBuffer() && 
        // "Can't call beginSwapChainRenderPass if freame is not in progress!");
    
    m_shadow_map_cmd_buf = commandBuffer;
    VkClearValue clear_values[1];
    clear_values[0].depthStencil.depth = 1.0f;
    clear_values[0].depthStencil.stencil = 0;
    
    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext = NULL;
    rp_begin.renderPass = m_renderPass;
    rp_begin.framebuffer = m_shadow_map_fb;
    rp_begin.renderArea.offset.x = 0;
    rp_begin.renderArea.offset.y = 0;
    rp_begin.renderArea.extent.width = SHADOW_MAP_WIDTH;
    rp_begin.renderArea.extent.height = SHADOW_MAP_HEIGHT;
    rp_begin.clearValueCount = 1;
    rp_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(commandBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport;
    viewport.height = SHADOW_MAP_HEIGHT;
    viewport.width = SHADOW_MAP_WIDTH;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(m_shadow_map_cmd_buf, 0, 1, &viewport);
    
    VkRect2D scissor;
    scissor.extent.width = SHADOW_MAP_WIDTH;
    scissor.extent.height = SHADOW_MAP_HEIGHT;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(m_shadow_map_cmd_buf, 0, 1, &scissor);
}

void shadowMappingSystem::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
    // assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
    // assert(commandBuffer == getCurrentCommandBuffer() && 
    //     "Can't end beginSwapChainRenderPass if freame is not in progress!");
    vkCmdEndRenderPass(commandBuffer);
}


} // namespace se
