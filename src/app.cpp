#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Systems/render_system.hpp"

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

App* App::s_Instance = nullptr;

App::App():
    m_window{800, 600, "Hydra"}
{
    assert(!s_Instance && "App already exists!");
    s_Instance = this;
 
    m_window.SetEventCallback(HY_BIND_EVENT_FN(App::onEvent));

    loadEntities();
}

App::~App(){}

void App::run(){
    RenderSystem renderSystem{m_device, m_renderer.getSwapChainRenderPass()};

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
            renderSystem.renderEntities(commandBuffer, m_registry);
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


} // namespace hyd
