#pragma once

#include "Core/Window.hpp" 
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"

namespace hyd
{
    
class App
{
public:
    App(/* args */);
    ~App();
    
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
};

} // namespace hyd
