#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Components/Viewer.hpp"

#include "systems/render_system.hpp"
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

    RenderSystem renderSystem{m_device, m_renderer};

    ViewerControllerSystem viewerControllerSystem{};
    
    const auto viewer_entity = m_registry.create();
    m_registry.emplace<ViewerComponent>(viewer_entity);
    m_registry.emplace<TransformComponent>(viewer_entity, 
        glm::vec3{0.f, -2.f, -2.5f},
        glm::vec3{0.5f, 0.5f, 0.5f},
        glm::vec3{0.f, 0.f, 0.f});

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

    Material material;
    material.name = std::string("TestMat");
    std::shared_ptr<Texture> texture = std::make_shared<Texture>(m_device, "../textures/dirt.jpg");
    material.textures.push_back(texture);
    m_materials[material.name] = material;

    std::shared_ptr<Model> model = Model::createModelFromFile(m_device, "../models/cube.obj");

    for (int i{0}; i<10; ++i){
        for (int j{0}; j<10; ++j){
            const auto entity = m_registry.create();
            m_registry.emplace<MeshComponent>(entity, model);
            m_registry.emplace<ColorComponent>(entity, glm::vec3(1.f, 1.f-0.1f*i, 0.2f+0.1f*j));
            m_registry.emplace<Material>(entity, m_materials["TestMat"]);
            m_registry.emplace<TransformComponent>(entity, 
                glm::vec3{-5+i*1.f, 0.f, -5+j*1.f},
                glm::vec3{1.f},
                glm::vec3{0.f, 0.f, 0.f});
        }
    }

    Material material2;
    texture = std::make_shared<Texture>(m_device, "../textures/dragon.png");
    material2.name = std::string("Dragon");
    material2.textures.push_back(texture);
    m_materials[material2.name] = material2;


    std::shared_ptr<Model> planeModel = Model::createModelFromFile(m_device, "../models/Dragon_wp4.obj");

    const auto entity = m_registry.create();
    m_registry.emplace<MeshComponent>(entity, planeModel);
    m_registry.emplace<Material>(entity, material2);
    m_registry.emplace<TransformComponent>(entity, 
    glm::vec3{0.f, 0.05f, 0.f},
    glm::vec3{1.f, 1.f, 1.f},
    glm::vec3{0.f, 0.f, 0.f});
}


} // namespace hyd
