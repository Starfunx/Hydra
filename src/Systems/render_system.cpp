#include "render_system.hpp"

#include "Renderer/SwapChain.hpp"

#include "Renderer/Buffer.hpp"

#include "Components/Transform.hpp"
#include "Components/Viewer.hpp"
#include "Renderer/Texture.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // two_pi
#include <glm/gtx/quaternion.hpp>

namespace hyd
{

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};

    glm::vec3 directionalLight{1.f, -1.f, 3.f};
    alignas(16) glm::vec4 ambiantLightColor{1.f, 1.f, 0.5f, 0.1f}; // w is light intensity
    
    glm::vec3 lightPosition{-1.f, -3.f, -1.f};
    alignas(16)glm::vec4 lightColor{1.f}; // w is light intensity
};


RenderSystem::RenderSystem(Device& device, Renderer& renderer)
: m_device{device}, m_renderer{renderer}
{
    // global descriptor pool
    globalPool =
    DescriptorPool::Builder(m_device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();


    // global descriptor set layout
    auto globalSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
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

    // write descriptors with buffers
    for (int i = 0; i < m_globalDescriptorSets.size(); i++) {
        auto bufferInfo = m_uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(m_globalDescriptorSets[i]);
    }

    // SUB RENDER SYSTEMS
    m_pointLightRenderSystem = std::make_unique<PointLightRenderSystem>(
        m_device,
        m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout());

    m_objectRenderSystem = std::make_unique<ObjectRenderSystem>(
        m_device,
        m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout());

    m_skyboxRenderSystem = std::make_unique<SkyboxRenderSystem>(
        m_device,
        m_renderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout());
        
    m_shadow_mapping_system = std::make_unique<shadowMappingSystem>(
        m_device,
        globalSetLayout->getDescriptorSetLayout());
        
    m_imageViewer = std::make_unique<ImageViewer>(
        m_device,
        m_renderer.getSwapChainRenderPass());

}

RenderSystem::~RenderSystem()
{
}

void RenderSystem::renderEntities(const float frameTime, entt::registry& registry)
{

    // get camera
    auto view = registry.view<TransformComponent, ViewerComponent>();
    Camera camera{};
    // camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.0f, 0.f, 1.f});
    camera.setViewTarget(glm::vec3{4.f}, glm::vec3{0.f});

    for(auto entity: view) {
        auto &transform = view.get<TransformComponent>(entity);
        // camera.setViewYXZ(transform.translation, glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f));
        // camera.setViewYXZ(transform.translation, glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f));
        camera.setViewQuat(transform.translation, transform.orientation);
    }


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
            m_globalDescriptorSets[frameIndex]};



        // update
        GlobalUbo ubo{};
        ubo.projection = camera.getProjection();
        ubo.view = camera.getView();
        m_uboBuffers[frameIndex]->writeToBuffer(&ubo);
        m_uboBuffers[frameIndex]->flush();

        // RENDER
        // shadow pass
        // m_renderer.beginSwapChainRenderPass(commandBuffer); // check if thoses would work
        m_shadow_mapping_system->beginSwapChainRenderPass(commandBuffer);
            m_shadow_mapping_system->renderEntities(frameInfo, registry);
        // m_renderer.endSwapChainRenderPass(commandBuffer);
        m_shadow_mapping_system->endSwapChainRenderPass(commandBuffer);

        // render
        m_renderer.beginSwapChainRenderPass(commandBuffer);        
            m_skyboxRenderSystem->render(frameInfo);
            m_objectRenderSystem->renderEntities(frameInfo, registry);
            m_pointLightRenderSystem->renderPointLightEntities(frameInfo);


            VkExtent2D extent2d{500, 400};
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(extent2d.width);
            viewport.height = static_cast<float>(extent2d.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, extent2d};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            // m_imageViewer->renderImage(frameInfo, m_shadow_mapping_system->getImage());
        m_renderer.endSwapChainRenderPass(commandBuffer);

        
        m_renderer.endFrame();
    }

}

} // namespace hyd
