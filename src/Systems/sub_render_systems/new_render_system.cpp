#include "new_render_system.hpp"

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
    glm::mat4 modelMatrix{1.f}; // projection * view * model
    glm::mat4 normalMatrix{1.f};
};

NewRenderSystem::NewRenderSystem(Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout):
m_device{device}{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

NewRenderSystem::~NewRenderSystem(){
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void NewRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

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

void NewRenderSystem::createPipeline(VkRenderPass renderPass){
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


void NewRenderSystem::renderEntities(
     FrameInfo& frameInfo,
     entt::registry& registry){
    auto view = registry.view<TransformComponent, MeshComponent, ColorComponent>();

    // bind pipline
    m_pipeline->bind(frameInfo.commandBuffer);

    // bind global descriptor set - at set #0
    vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

    // sorting by materials
    // registry.sort<renderable>([](const auto &lhs, const auto &rhs) {
    //     return lhs.z < rhs.z;
    // });



    // for each Materials
        // bind material descriptor set - at set #1

        // for each obj that uses the material
            // bind object descriptor set - at set #2
            // bind obj model
            // draw object

    for(auto entity: view) {
        auto &transform = view.get<TransformComponent>(entity);
        auto &mesh = view.get<MeshComponent>(entity);
        auto &color = view.get<ColorComponent>(entity);

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

        mesh.model->bind(frameInfo.commandBuffer);
        mesh.model->draw(frameInfo.commandBuffer);
    }
}

} // namespace se
