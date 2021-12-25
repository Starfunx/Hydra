#include "app.hpp"

#include "Core/Input.hpp"
// std
#include <iostream>
#include <cassert>
#include <stdexcept>

namespace hyd
{

App* App::s_Instance = nullptr;

App::App():
    m_window{800, 600, "Hydra"}
{
    assert(!s_Instance && "App already exists!");
    s_Instance = this;
 
    m_window.SetEventCallback(HY_BIND_EVENT_FN(App::onEvent));


    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

App::~App()
{
    vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void App::run()
{
    while (!m_shouldEnd)
    {
        glfwPollEvents();
        if (Input::IsKeyPressed(key::Space))
        {
            /* code */
            // std::cout << "Space" << std::endl;
        }
        

    }
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

return true;
}




void App::createPipelineLayout(){
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("failed to create pipeline layout");
    }

}

void App::createPipeline(){
    auto pipelineConfig = Pipeline::defaultPipelineConfigInfo(m_swapChain.width(), m_swapChain.height());
    pipelineConfig.renderPass = m_swapChain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_pipeline = std::make_unique<Pipeline>(
        m_device,
        "../shaders/simple_shader.vert.spv",
        "../shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void App::createCommandBuffers(){

}

void App::drawFrame(){

}


} // namespace hyd
