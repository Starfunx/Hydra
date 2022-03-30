cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target x86_64-w64-mingw32") #force Clang to use its own libraries instead of MSVC's
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-w64-mingw32") #force Clang to use its own libraries instead of MSVC's

project(Hydra VERSION 0.1)

set(CMAKE_CXX_STANDARD 20) # c++ standard: c++20
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(VENDOR_DIR "${PROJECT_SOURCE_DIR}/vendor")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)


set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/app.cpp

    ${SRC_DIR}/Core/Window.cpp
    ${SRC_DIR}/Core/Input.cpp

    ${SRC_DIR}/Ressources/Image.cpp
    ${SRC_DIR}/Managers/TextureManager.cpp
    ${SRC_DIR}/Managers/MeshManager.cpp
    ${SRC_DIR}/Managers/MaterialManager.cpp

    ${SRC_DIR}/Renderer/Device.cpp
    ${SRC_DIR}/Renderer/Pipeline.cpp
    ${SRC_DIR}/Renderer/SwapChain.cpp
    ${SRC_DIR}/Renderer/Model.cpp
    ${SRC_DIR}/Renderer/Renderer.cpp
    ${SRC_DIR}/Renderer/Camera.cpp
    ${SRC_DIR}/Renderer/Buffer.cpp
    ${SRC_DIR}/Renderer/DescriptorSet.cpp
    ${SRC_DIR}/Renderer/Texture.cpp

    ${SRC_DIR}/Systems/viewer_controller.cpp
    
    ${SRC_DIR}/Systems/render_system.cpp
    ${SRC_DIR}/Systems/sub_render_systems/point_light_render_system.cpp
    ${SRC_DIR}/Systems/sub_render_systems/object_render_system.cpp
    ${SRC_DIR}/Systems/sub_render_systems/skybox_render_system.cpp
    ${SRC_DIR}/Systems/sub_render_systems/shadowMappingSystem.cpp
    ${SRC_DIR}/Systems/sub_render_systems/imageViewer.cpp
  )


add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

#compiler flags for our defined target
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE 
    # -pedantic-errors    # disable extentions
    # -Wall               # warning level to max
    # -Weffc++
    # -Wextra
    # -Wsign-conversion
    # -Werror             # treat warnings as errors
)

##########################################################
#VULKAN SDK
# set(VULKAN_SDK_PATH "C:/VulkanSDK/1.2.170.0")
# set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
# set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct

find_package(Vulkan REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Vulkan::Vulkan)

#########################################################
#GLFW

set(GLFW_PATH "${PROJECT_SOURCE_DIR}/vendor/glfw")
set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")

add_subdirectory(${GLFW_PATH})
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} glfw ${GLFW_LIBRARIES})

###########################################################
# GLM
add_subdirectory(${VENDOR_DIR}/glm)
target_link_libraries(${CMAKE_PROJECT_NAME} glm)

###########################################################
# tiny glTf
include_directories(${VENDOR_DIR}/tinygltf)

###########################################################
# TinyObj
include_directories(${VENDOR_DIR}/tinyobjloader)

#############################################################
# Entt
target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC 
        ${VENDOR_DIR}/entt/src/
)
#############################################################
#stb
include_directories(${VENDOR_DIR}/stb)

#############################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    # ${PROJECT_BINARY_DIR} # for cmaked configured headers
    ${SRC_DIR}

    # ${Vulkan_INCLUDE_DIRS}
    # ${GLFW_INCLUDE_DIRS}
)

target_link_directories(${PROJECT_NAME} PUBLIC
    # ${Vulkan_LIBRARIES}
    # ${GLFW_LIB}
)


############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

