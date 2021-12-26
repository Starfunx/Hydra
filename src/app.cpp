#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Components/Viewer.hpp"

#include "Systems/render_system.hpp"
#include "Systems/viewer_controller.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // two_pi

// std
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <chrono>

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
    ViewerControllerSystem viewerControllerSystem{};
    Camera camera{};
    camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.0f, 0.f, 1.f});

    const auto viewer_entity = m_registry.create();
    m_registry.emplace<ViewerComponent>(viewer_entity);
    m_registry.emplace<TransformComponent>(viewer_entity, 
        glm::vec3{0.f, 0.f, 1.5f},
        glm::vec3{0.5f, 0.5f, 0.5f},
        glm::vec3{0.f, 0.f, 0.f});

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!m_shouldEnd)
    {
        glfwPollEvents();
        if (Input::IsKeyPressed(key::Space))
        {
            /* code */
            // std::cout << "Space" << std::endl;
        }
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        
        viewerControllerSystem.moveInPlaneXZ(frameTime, m_registry);

        auto view = m_registry.view<TransformComponent, ViewerComponent>();
        for(auto entity: view) {
            auto &transform = view.get<TransformComponent>(entity);
            camera.setViewYXZ(transform.translation, transform.rotation);
        }

        frameTime = std::min(frameTime, 0.5f);
        

        float aspect = m_renderer.getAspectRatio();
        // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

        if (auto commandBuffer = m_renderer.beginFrame()){
     
            // beigin offscreen shadow pass
            // render shadow vasting objects
            // end offscreen shadow pass
     
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            renderSystem.renderEntities(commandBuffer, m_registry, camera);
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
    std::shared_ptr<Model> model = Model::createModelFromFile(m_device, "../models/smooth_vase.obj");

    const auto entity = m_registry.create();
    m_registry.emplace<MeshComponent>(entity, model);
    m_registry.emplace<ColorComponent>(entity, glm::vec3(1.f, 0.f, 0.f));
    m_registry.emplace<TransformComponent>(entity, 
        glm::vec3{0.f, 0.f, 1.5f},
        glm::vec3{3.f},
        glm::vec3{0.f, 0.f, 0.f});
}


} // namespace hyd
