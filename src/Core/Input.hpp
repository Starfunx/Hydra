#pragma once
#include "KeyCodes.hpp"
#include "MouseCodes.hpp"

#include <GLFW/glfw3.h>

#include <utility>

namespace hyd {

class Input
{
public:
    static bool IsKeyPressed(KeyCode key);

    static bool IsMouseButtonPressed(MouseCode button);
    static std::pair<float, float> GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
};

}