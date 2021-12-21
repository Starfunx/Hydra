#include "app.hpp"

#include "Core/Input.hpp"
// std
#include <iostream>
#include <cassert>

namespace hyd
{

App* App::s_Instance = nullptr;

App::App():
    m_window{800, 600, "Hydra"}
{

    assert(!s_Instance && "App already exists!");
    s_Instance = this;
}

App::~App()
{
}

void App::run()
{
    while (!m_window.shouldClose())
    {
        glfwPollEvents();
        if (Input::IsKeyPressed(key::Space))
        {
            /* code */
            std::cout << "Space" << std::endl;
        }
        

    }
}

} // namespace hyd
