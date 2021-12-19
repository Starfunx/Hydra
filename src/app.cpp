#include "app.hpp"

#include <iostream>

namespace hyd
{

App::App():m_window{800, 600, "Hydra"}
{}

App::~App()
{
}

void App::run()
{
    while (!m_window.shouldClose())
    {
        glfwPollEvents();

    }
}

} // namespace hyd
