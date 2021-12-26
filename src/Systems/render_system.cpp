#include "render_system.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"

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

struct SimplePushConstantData {
    glm::mat4 transform{1.f};
    alignas(16) glm::vec3 color;
};

RenderSystem::RenderSystem(Device& device, VkRenderPass renderPass):
m_device{device}{
    createPipelineLayout();
    createPipeline(renderPass);
}
RenderSystem::~RenderSystem(){
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void RenderSystem::createPipelineLayout(){

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to create pipeline layout");
    }

}

void RenderSystem::createPipeline(VkRenderPass renderPass){
    assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        "../shaders/simple_shader.vert.spv",
        "../shaders/simple_shader.frag.spv",
        pipelineConfig);
}


void RenderSystem::renderEntities(
    VkCommandBuffer commandBuffer,
     entt::registry& registry,
     const Camera& camera ){
    auto view = registry.view<TransformComponent, MeshComponent, ColorComponent>();

    auto projectionView = camera.getProjection()*camera.getView();

    m_pipeline->bind(commandBuffer);

    for(auto entity: view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &mesh = view.get<MeshComponent>(entity);
        auto &color = view.get<ColorComponent>(entity);


        transform.rotation.y = glm::mod(transform.rotation.y + 0.0005f, glm::two_pi<float>());
        transform.rotation.x = glm::mod(transform.rotation.y + 0.00025f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.transform = projectionView*transform.mat4();
        push.color = color.m_color;

        vkCmdPushConstants(
            commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);

        mesh.model->bind(commandBuffer);
        mesh.model->draw(commandBuffer);
    }

}

} // namespace se
