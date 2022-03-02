#include "object_render_system.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Components/Material.hpp"
#include "Components/Viewer.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace hyd
{

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};

    glm::mat4 lightMVP{1.f};
    glm::vec3 directionalLight{1.f, 1.f, -2.f};
    alignas(16) glm::vec4 ambiantLightColor{1.f, 1.f, 0.5f, 0.1f}; // w is light intensity
    
    glm::vec3 lightPosition{0.f, 0.f, 0.f};
    alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
};
struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f}; // projection * view * model
    glm::mat4 normalMatrix{1.f};
};


ObjectRenderSystem::ObjectRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkImageView imageView):
m_device{device}{

    m_globalPool =
    DescriptorPool::Builder(m_device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    m_objectPool = 
    DescriptorPool::Builder(m_device)
        .setMaxSets(1000) // large amount alocated
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .build();

    // allocate descriptor sets from the descriptor pool
    m_materialSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    // global descriptor set layout
    m_globalSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();


    // create buffers
    for (int i = 0; i < m_uboBuffers.size(); i++) {
        m_uboBuffers[i] = std::make_unique<Buffer>(
            m_device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        m_uboBuffers[i]->map();
    }

    // Create sampler to sample from to depth attachment
    // Used to sample in the fragment shader for shadowed rendering
    VkFilter shadowmap_filter = VK_FILTER_NEAREST;
    VkSamplerCreateInfo samplerCI {};
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.maxAnisotropy = 1.0f;
    samplerCI.magFilter = shadowmap_filter;
    samplerCI.minFilter = shadowmap_filter;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCI.addressModeV = samplerCI.addressModeU;
    samplerCI.addressModeW = samplerCI.addressModeU;
    samplerCI.mipLodBias = 0.0f;
    samplerCI.maxAnisotropy = 1.0f;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = 1.0f;
    samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vkCreateSampler(m_device.device(), &samplerCI, nullptr, &m_sampler);


    // Image descriptor for the shadow map attachment
    VkDescriptorImageInfo descriptorImageInfo {};
    descriptorImageInfo.sampler = m_sampler;
    descriptorImageInfo.imageView = imageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // write descriptors with buffers
    for (int i = 0; i < m_globalDescriptorSets.size(); i++) {
        auto bufferInfo = m_uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*m_globalSetLayout, *m_globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &descriptorImageInfo)
            .build(m_globalDescriptorSets[i]);
    }

    createPipelineLayout(globalSetLayout, m_materialSetLayout->getDescriptorSetLayout());
    createPipeline(renderPass);
}

ObjectRenderSystem::~ObjectRenderSystem(){
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void ObjectRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout objectSetLayout) {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{m_globalSetLayout->getDescriptorSetLayout(), objectSetLayout};

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

void ObjectRenderSystem::createPipeline(VkRenderPass renderPass){
    assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        "../shaders/new_shader.vert.spv",
        "../shaders/new_shader.frag.spv",
        pipelineConfig);
}


void ObjectRenderSystem::renderEntities(
     FrameInfo& frameInfo,
     entt::registry& registry,
     const glm::mat4& lightDepthMVP,
     VkImageView imageView,
     float aspectRatio){

    // get camera
    Camera camera;
    auto viewcam = registry.view<TransformComponent, ViewerComponent>();
    for(auto entity: viewcam) {
        auto &transform = viewcam.get<TransformComponent>(entity);
        camera.setViewQuat(transform.translation, transform.orientation);
    }
    // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
    camera.setPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1f, 100.f);

    auto view = registry.view<TransformComponent, MeshComponent, Material>();

    // bind pipline
    m_pipeline->bind(frameInfo.commandBuffer);


    // bind global descriptor set - at set #0
    GlobalUbo ubo{};
    ubo.projection = camera.getProjection();
    ubo.view = camera.getView();
    ubo.lightMVP = lightDepthMVP;
    m_uboBuffers[frameInfo.FrameIndex]->writeToBuffer(&ubo);
    m_uboBuffers[frameInfo.FrameIndex]->flush();

    vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &m_globalDescriptorSets[frameInfo.FrameIndex],
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
                frameInfo.commandBuffer,
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
                frameInfo.commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            // bind obj model
            mesh.model->bind(frameInfo.commandBuffer);
            // draw object
            mesh.model->draw(frameInfo.commandBuffer);
    }
}

} // namespace se
