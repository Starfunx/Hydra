#include "app.hpp"

#include "Core/Input.hpp"

#include "Components/Color.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "Components/Viewer.hpp"

#include "Systems/render_system.hpp"
#include "systems/point_light_render_system.hpp"

#include "Systems/viewer_controller.hpp"


#include "Renderer/Buffer.hpp"
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

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambiantLightColor{1.f, 1.f, 1.f, 0.2f}; // w is light intensity
    glm::vec3 lightPosition{-1.f, -3.f, -1.f};
    alignas(16)glm::vec4 lightColor{1.f}; // w is light intensity
};


App* App::s_Instance = nullptr;

App::App():
    m_window{800, 600, "Hydra"}
{
    assert(!s_Instance && "App already exists!");
    s_Instance = this;
 
    m_window.SetEventCallback(HY_BIND_EVENT_FN(App::onEvent));


    globalPool =
    DescriptorPool::Builder(m_device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    loadEntities();
}

App::~App(){}

void App::run(){

    std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<Buffer>(
            m_device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    auto globalSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    RenderSystem renderSystem{
        m_device,
        m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()};

    PointLightRenderSystem pointLightRenderSystem{
        m_device,
        m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()};

    ViewerControllerSystem viewerControllerSystem{};
    Camera camera{};
    camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.0f, 0.f, 1.f});

    const auto viewer_entity = m_registry.create();
    m_registry.emplace<ViewerComponent>(viewer_entity);
    m_registry.emplace<TransformComponent>(viewer_entity, 
        glm::vec3{0.f, 0.f, -2.5f},
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
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = m_renderer.beginFrame()){
            int frameIndex = m_renderer.getFrameIndex();

            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex]};



            // update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // RENDER
            
            // beigin offscreen shadow pass
            // render shadow vasting objects
            // end offscreen shadow pass
     
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            
                renderSystem.renderEntities(frameInfo, m_registry);
                
                // don't care about entities, just render the only point light in ubo
                pointLightRenderSystem.renderPointLightEntities(frameInfo, m_registry);
           
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

    for (int i{0}; i<10; ++i){
        for (int j{0}; j<10; ++j){

            const auto entity = m_registry.create();
            m_registry.emplace<MeshComponent>(entity, model);
            m_registry.emplace<ColorComponent>(entity, glm::vec3(1.f, 0.f, 0.f));
            m_registry.emplace<TransformComponent>(entity, 
                glm::vec3{-5+i*1.f, 0.f, -5+j*1.f},
                glm::vec3{3.f},
                glm::vec3{0.f, 0.f, 0.f});
        }
    }
    std::shared_ptr<Model> planeModel = Model::createModelFromFile(m_device, "../models/quad.obj");

    const auto entity = m_registry.create();
    m_registry.emplace<MeshComponent>(entity, planeModel);
    m_registry.emplace<ColorComponent>(entity, glm::vec3(1.f, 0.f, 0.f));
    m_registry.emplace<TransformComponent>(entity, 
    glm::vec3{0.f, 0.f, 0.f},
    glm::vec3{3.f},
    glm::vec3{0.f, 0.f, 0.f});
}


} // namespace hyd
