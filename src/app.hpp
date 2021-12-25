#pragma once

#include "Core/Window.hpp" 
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

#include "Renderer/Device.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/SwapChain.hpp"
#include "Renderer/Model.hpp"

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

    void loadModels();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();
    void recreateSwapChain();
    void recordCommandBuffer(int imageeIndex);
    void freeCommandBuffers();

    /* data */
    Window m_window;

    bool m_shouldEnd{false};
    static App* s_Instance;

    Device m_device{m_window};
    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<Pipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::unique_ptr<Model> m_model;

    bool m_windowResized{false};
};

} // namespace hyd
