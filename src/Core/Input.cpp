#include "Input.hpp"
#include "app.hpp"

namespace hyd
{

bool Input::IsKeyPressed(KeyCode key)
{
    auto* window = static_cast<GLFWwindow*>(App::Get().GetWindow().getGLFWwindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
    auto* window = static_cast<GLFWwindow*>(App::Get().GetWindow().getGLFWwindow());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::GetMousePosition()
{
    auto* window = static_cast<GLFWwindow*>(App::Get().GetWindow().getGLFWwindow());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    return { (float)xpos, (float)ypos };
}

float Input::GetMouseX()
{
    auto[x, y] = GetMousePosition();
    return x;
}

float Input::GetMouseY()
{
    auto[x, y] = GetMousePosition();
    return y;
}
    
} // namespace hyd
