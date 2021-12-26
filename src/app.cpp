#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // two_pi

// std
#include <iostream>
#include <cassert>
#include <stdexcept>

namespace hyd
{


struct SimplePushConstantData {
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};


App* App::s_Instance = nullptr;

App::App():
    m_window{800, 600, "Hydra"}
{
    assert(!s_Instance && "App already exists!");
    s_Instance = this;
 
    m_window.SetEventCallback(HY_BIND_EVENT_FN(App::onEvent));

    loadEntities();
    createPipelineLayout();
    createPipeline();
}

App::~App(){
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void App::run(){
    while (!m_shouldEnd)
    {
        glfwPollEvents();
        if (Input::IsKeyPressed(key::Space))
        {
            /* code */
            // std::cout << "Space" << std::endl;
        }

        if (auto commandBuffer = m_renderer.beginFrame()){
     
            // beigin offscreen shadow pass
            // render shadow vasting objects
            // end offscreen shadow pass
     
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            renderEntities(commandBuffer);
            m_renderer.endSwapChainRenderPass(commandBuffer);
            
            m_renderer.endFrame();
        }
        
    }
    vkDeviceWaitIdle(m_device.device());
}

void App::onEvent(Event& e){

    // std::cout << e << std::endl;


    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<WindowCloseEvent>(HY_BIND_EVENT_FN(App::OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(HY_BIND_EVENT_FN(App::OnWindowResize));

}


bool App::OnWindowClose(WindowCloseEvent& e){
    m_shouldEnd = true;
    return false;
}

bool App::OnWindowResize(WindowResizeEvent& e){
    m_renderer.setFrameBufferResized();
    return true;
}


void App::loadEntities(){
    std::vector<Model::Vertex> vertices {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    auto model = std::make_shared<Model> (m_device, vertices);

    const auto entity = m_registry.create();
    m_registry.emplace<MeshComponent>(entity, model);
    m_registry.emplace<ColorComponent>(entity, glm::vec3(1.f, 0.f, 0.f));
    m_registry.emplace<Transform2dComponent>(entity, 
        glm::vec2(0.f, 0.f),
        glm::vec2(1.f, 1.f),
        0.f);

}


void App::createPipelineLayout(){
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

void App::createPipeline(){
    assert(m_pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = m_renderer.getSwapChainRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        "../shaders/simple_shader.vert.spv",
        "../shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void App::renderEntities(VkCommandBuffer commandBuffer){
    auto view = m_registry.view<Transform2dComponent, MeshComponent, ColorComponent>();
    
    m_pipeline->bind(commandBuffer);

    for(auto entity: view) {
        auto &transform = view.get<Transform2dComponent>(entity);
        auto &mesh = view.get<MeshComponent>(entity);
        auto &color = view.get<ColorComponent>(entity);

        transform.rotation = glm::mod(transform.rotation + 0.001f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.transform = transform.mat2();
        push.offset = transform.translation;
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

} // namespace hyd
