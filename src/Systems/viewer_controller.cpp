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


        glm::quat orientation = transform.orientation;

        glm::vec3 rotate{0};
        if (Input::IsKeyPressed(keys.lookLeft)) rotate.z += 1.f;// yaw
        if (Input::IsKeyPressed(keys.lookRight)) rotate.z -= 1.f;  
        if (Input::IsKeyPressed(keys.lookUp)) rotate.x -= 1.f;  // pitch
        if (Input::IsKeyPressed(keys.lookDown)) rotate.x += 1.f;
        if (Input::IsKeyPressed(keys.rollLeft)) rotate.y += 1.f;// roll
        if (Input::IsKeyPressed(keys.rollRight)) rotate.y -= 1.f; 

        glm::quat rotYaw {glm::angleAxis(glm::degrees(rotate.z/(float)3e3), glm::vec3(0.0f,1.0f,0.0f))};
        glm::quat rotPitch {glm::angleAxis(glm::degrees(rotate.x/(float)3e3), glm::vec3(1.0f,0.0f,0.0f))};
        glm::quat rotRoll {glm::angleAxis(glm::degrees(rotate.y/(float)3e3), glm::vec3(0.0f,0.0f,1.0f))};

        orientation = rotRoll * rotPitch * rotYaw * orientation;
        transform.orientation = glm::normalize(orientation);

        glm::vec3 cameraFront {glm::conjugate(orientation) * glm::vec3 (0.0f, 0.0f, -1.0f)};
        glm::vec3 cameraUp {glm::conjugate(orientation) * glm::vec3 (0.0f, 1.0f, 0.0f)};
        glm::vec3 cameraRight {glm::conjugate(orientation) * glm::vec3 (-1.0f, 0.0f, 0.0f)};

        glm::vec3 moveDir{0};
        if (Input::IsKeyPressed(keys.moveForward)) moveDir -= cameraFront;
        if (Input::IsKeyPressed(keys.moveBackward)) moveDir += cameraFront;
        if (Input::IsKeyPressed(keys.moveLeft)) moveDir += cameraRight;
        if (Input::IsKeyPressed(keys.moveRight)) moveDir -= cameraRight;
        if (Input::IsKeyPressed(keys.moveUp)) moveDir -= cameraUp;
        if (Input::IsKeyPressed(keys.moveDown)) moveDir += cameraUp;

        if(glm::length(moveDir) > std::numeric_limits<float>::epsilon()){
            transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

}

    
} // namespace se
