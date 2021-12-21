#pragma once

#include "Core/Window.hpp"

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
    /* data */
    Window m_window;

    bool m_shouldEnd{false};

    static App* s_Instance;
};


} // namespace hyd
