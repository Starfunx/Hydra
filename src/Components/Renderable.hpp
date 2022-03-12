/*
The transform component gives an object a position and orientation in space!
*/
#pragma once

#include "Renderer/Model.hpp"
#include "Renderer/Material.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace hyd
{
    
struct RenderableComponent
{
   std::shared_ptr<Model> model {nullptr};
   std::shared_ptr<Material> material {nullptr};
};

} // namespace hyd

