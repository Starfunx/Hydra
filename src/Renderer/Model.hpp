/*
This class takes vertex data in file (on cpu) and allocate the memory
and copy the data on the device's GPU so it can be rendered effeciently
*/
#pragma once

#include "Device.hpp"
#include "Buffer.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
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
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributesDescriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }

    };
    
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        
        void loadModel(const std::string& filepath);
    };
    
    
    Model(Device& device, const Model::Builder &builder);
    ~Model();

    Model(const Model&) = delete;
    Model &operator=(const Model&) = delete;

    static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filepath);

    void bind(VkCommandBuffer VkCommandBuffer);
    void draw(VkCommandBuffer VkCommandBuffer);


private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    /* data */
    Device& m_device;

    std::unique_ptr<Buffer> m_vertexBuffer;
    uint32_t m_vertexCount;

    bool m_hasIndexBuffer = false;
    std::unique_ptr<Buffer> m_indexBuffer;
    uint32_t m_indexCount;
};

}