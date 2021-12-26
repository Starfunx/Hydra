#include "viewer_controller.hpp"

#include "Components/Transform.hpp"
#include "Components/Viewer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace hyd
{

void ViewerControllerSystem::moveInPlaneXZ(float dt, entt::registry& registry){

    auto view = registry.view<TransformComponent, ViewerComponent>();

    for(auto entity: view) {
        auto &transform = view.get<TransformComponent>(entity);

        glm::vec3 rotate{0};
        if (Input::IsKeyPressed(keys.lookRight)) rotate.y += 1.f;
        if (Input::IsKeyPressed(keys.lookLeft)) rotate.y -= 1.f;
        if (Input::IsKeyPressed(keys.lookUp)) rotate.x += 1.f;
        if (Input::IsKeyPressed(keys.lookDown)) rotate.x -= 1.f;

        if(glm::length(rotate) > std::numeric_limits<float>::epsilon()){
            transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        //limit pitch between +-85degrees
        transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

        float yaw = transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 updDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0};
        if (Input::IsKeyPressed(keys.moveForward)) moveDir += forwardDir;
        if (Input::IsKeyPressed(keys.moveBackward)) moveDir -= forwardDir;
        if (Input::IsKeyPressed(keys.moveLeft)) moveDir -= rightDir;
        if (Input::IsKeyPressed(keys.moveRight)) moveDir += rightDir;
        if (Input::IsKeyPressed(keys.moveUp)) moveDir += updDir;
        if (Input::IsKeyPressed(keys.moveDown)) moveDir -= updDir;

        if(glm::length(moveDir) > std::numeric_limits<float>::epsilon()){
            transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

}

    
} // namespace se
