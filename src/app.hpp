#pragma once

#include "Core/Window.hpp" 
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/DescriptorSet.hpp"

#include "Managers/TextureManager.hpp"
#include "Managers/MeshManager.hpp"
#include "Managers/MaterialManager.hpp"

//libs
#include <entt/entt.hpp>

// std
#include <memory>
#include <vector>
#include <unordered_map>

namespace hyd
{
    
class App
{
public:
    App(/* args */);
    ~App();
    
    static constexpr int WIDTH = 1080;
    static constexpr int HEIGHT = 720;


    void run();

    void onEvent(Event& e);

    static App& Get() { return *s_Instance; }
    Window& GetWindow() { return m_window; }

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

    void loadEntities();

    /* data */
    Window m_window;

    bool m_shouldEnd{false};
    static App* s_Instance;

    Device m_device{m_window};
    Renderer m_renderer{m_window, m_device};

    DescriptorLayoutCache m_cache{m_device};
    DescriptorAllocator m_alloc{m_device};

    TextureManager m_textureManager{m_device};
    MaterialManager m_materialManager{m_device, m_cache, m_alloc, m_textureManager};
    MeshManager m_meshManager{m_device};

    entt::registry m_registry;
};

} // namespace hyd
