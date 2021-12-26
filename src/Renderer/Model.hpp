/*
This class takes vertex data in file (on cpu) and allocate the memory
and copy the data on the device's GPU so it can be rendered effeciently
*/
#pragma once

#include "Device.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace hyd {

class Model
{
public:
    struct Vertex
    {
        /* data */
        glm::vec3 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributesDescriptions();

    };
    
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
    };
    
    
    Model(Device& device, const Model::Builder &builder);
    ~Model();

    Model(const Model&) = delete;
    Model &operator=(const Model&) = delete;

    void bind(VkCommandBuffer VkCommandBuffer);
    void draw(VkCommandBuffer VkCommandBuffer);


private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);
    
    /* data */
    Device& m_device;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    uint32_t m_vertexCount;

    bool m_hasIndexBuffer = false;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    uint32_t m_indexCount;
};

}