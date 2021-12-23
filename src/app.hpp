#pragma once

#include "Core/Window.hpp" 
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

#include "Renderer/Device.hpp"
#include "Renderer/Pipeline.hpp"

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
    /* data */
    Window m_window;

    bool m_shouldEnd{false};
    static App* s_Instance;

    Device m_device{m_window};
    Pipeline m_pipeline{
        m_device,
        "../shaders/simple_shader.vert.spv",
        "../shaders/simple_shader.frag.spv", 
        Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};

};

} // namespace hyd
