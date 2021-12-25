#include "Model.hpp"

// std
#include <cassert>

namespace hyd
{

Model::Model(Device& device, const std::vector<Vertex> &vertices):
m_device{device}{
    createVertexBuffers(vertices);
}

Model::~Model()
{
    vkDestroyBuffer(m_device.device(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_device.device(), m_vertexBufferMemory, nullptr);
}


void Model::bind(VkCommandBuffer commandBuffer){
    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

}

void Model::draw(VkCommandBuffer commandBuffer){
    vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);

}


void Model::createVertexBuffers(const std::vector<Vertex> &vertices){
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3"); // at least 1 triangle
    VkDeviceSize bufferSize{sizeof(vertices[0]) * m_vertexCount};

    m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_vertexBuffer,
        m_vertexBufferMemory);

    void* data;
    vkMapMemory(m_device.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));

    vkUnmapMemory(m_device.device(), m_vertexBufferMemory);

}


std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;    
    bindingDescriptions[0].stride = sizeof(Vertex);    
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributesDescriptions(){
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    return attributeDescriptions;
}


}
