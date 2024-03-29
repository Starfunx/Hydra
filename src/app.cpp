#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Transform.hpp"
#include "Components/Renderable.hpp"
#include "Components/Camera.hpp"

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
    m_window{WIDTH, HEIGHT, "Hydra"}
{
    assert(!s_Instance && "App already exists!");
    s_Instance = this;
 
    m_window.SetEventCallback(HY_BIND_EVENT_FN(App::onEvent));

    loadEntities();
}

App::~App(){}

void App::run(){

    RenderSystem renderSystem{m_device, m_renderer};

    ViewerControllerSystem viewerControllerSystem{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!m_shouldEnd)
    {
        glfwPollEvents();
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        frameTime = std::min(frameTime, 0.5f);
        
        viewerControllerSystem.moveInPlaneXZ(frameTime, m_registry);

        renderSystem.renderEntities(frameTime, m_registry);
        
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

    { // load textures
        m_textureManager.loadRessource("../textures/dirt.jpg");
    }
    { // load materials
        m_materialManager.loadRessource("../textures/dirt.jpg");
    }
    { // load meshs
        m_meshManager.loadRessource("../models/cube.gltf");
        // m_meshManager.loadRessource("../models/cube.obj");
    }

    // {    
    //     std::shared_ptr<Model> model = Model::createModelFromFile(m_device, "../models/cube.obj");
    // }

    // camera 
    {
       const auto entity = m_registry.create();
       auto& camera = m_registry.emplace<CameraComponent>(entity);
       camera.position = glm::vec3(1.0f, 1.0f, 3.0f);
       camera.orientation = glm::quat(glm::vec3{glm::half_pi<float>(), 0.f, 0.0f});
    }

    // cubes
    {
        for(int i{-5}; i<4; ++i)
        {  
            for(int j{-5}; j<4; ++j)
            {  
                const auto entity = m_registry.create();
                auto& pos = m_registry.emplace<TransformComponent>(entity);
                pos.translation = glm::vec3{j*1.f, i*1.f, 0.f};
                auto& renderable = m_registry.emplace<RenderableComponent>(entity);
                renderable.material = m_materialManager.getRessource("../textures/dirt.jpg");
                renderable.model = m_meshManager.getRessource("../models/cube.gltf");
            }
        }
    }




}


} // namespace hyd
