#pragma once

#include "Core/Window.hpp" 
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"

//libs
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>

namespace hyd
{
    
class App
{
public:
    App(/* args */);
    ~App();
    
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;


    void run();

    void onEvent(Event& e);

    static App& Get() { return *s_Instance; }
    Window& GetWindow() { return m_window; }

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    void loadEntities();
    void createPipelineLayout();
    void createPipeline();
    void renderEntities(VkCommandBuffer commandBuffer);

    /* data */
    Window m_window;

    bool m_shouldEnd{false};
    static App* s_Instance;

    Device m_device{m_window};
    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    Renderer m_renderer{m_window, m_device};

    entt::registry m_registry;
};

} // namespace hyd
