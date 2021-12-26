#pragma once

#include "Core/Input.hpp"

// libs
#include <entt/entt.hpp>

namespace hyd
{
    
    class ViewerControllerSystem
    {
    public:
        struct KeyMappings {
            int moveLeft = key::A;
            int moveRight = key::D;
            int moveForward = key::W;
            int moveBackward = key::S;
            int moveUp = key::Space;
            int moveDown = key::LeftShift;
            int lookLeft = key::Left;
            int lookRight = key::Right;
            int lookUp = key::Up;
            int lookDown = key::Down;
        };

        void moveInPlaneXZ(float dt, entt::registry& registry);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    private:
        /* data */

    };
    

    


} // namespace se
