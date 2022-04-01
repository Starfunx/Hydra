#include "Model.hpp"

#include "Utils.hpp"

// libs
#include "stb_image.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE 
#include "tiny_gltf.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

// std
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace std {
template<>
struct hash<hyd::Model::Vertex> {
  size_t operator()(hyd::Model::Vertex const &vertex) const {
    size_t seed = 0;
    hyd::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};


}
namespace hyd
{

Model::Model(Device& device, const Model::Builder &builder):
m_device{device}{
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

Model::~Model(){}


std::unique_ptr<Model> Model::createModelFromFile(
    Device& device, const std::string& filepath){
    Builder builder{};

    if(filepath.substr(filepath.find_last_of(".") + 1) == "obj")
        builder.loadOBJModel(filepath);

    else if(filepath.substr(filepath.find_last_of(".") + 1) == "gltf")
        builder.loadGLTFModel(filepath);

    else 
        throw std::runtime_error("unable to open file" + filepath);

    return std::make_unique<Model>(device, builder);
}


void Model::bind(VkCommandBuffer commandBuffer){
    VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    
    if (m_hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void Model::draw(VkCommandBuffer commandBuffer){
    if (m_hasIndexBuffer){
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }
}


void Model::createVertexBuffers(const std::vector<Vertex> &vertices){
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3"); // at least 1 triangle
    VkDeviceSize bufferSize{sizeof(vertices[0]) * m_vertexCount};
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer{
        m_device,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };


    stagingBuffer.map();
    stagingBuffer.writeToBuffer( (void*) vertices.data());

    m_vertexBuffer = std::make_unique<Buffer>(
        m_device,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t> &indices){
    m_indexCount = static_cast<uint32_t>(indices.size());
    m_hasIndexBuffer = m_indexCount > 0;

    if (!m_hasIndexBuffer){
        return;
    }
    VkDeviceSize bufferSize{sizeof(indices[0]) * m_indexCount};
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer{
        m_device,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer( (void*) indices.data());

    m_indexBuffer = std::make_unique<Buffer>(
        m_device,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
}


std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;    
    bindingDescriptions[0].stride = sizeof(Vertex);    
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributesDescriptions(){
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    //                              {location, binding, format, offset}
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    
    return attributeDescriptions;
}


void Model::Builder::loadOBJModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

  
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
    throw std::runtime_error(warn + err);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, uint32_t> uniqueVertices{};
  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};

      if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    }
  }
}



void Model::Builder::loadGLTFModel(const std::string &filepath) {
    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filepath);


    vertices.clear(); // TODO for animated model update depending on vertex attributes ???
    indices.clear();
	if (fileLoaded) {
            
        const tinygltf::Scene& scene = glTFInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];

            
            // If the node contains mesh data, we load vertices and indices from the buffers
            // In glTF this is done via accessors and buffer views
            if (node.mesh > -1) {
                const tinygltf::Mesh mesh = glTFInput.meshes[node.mesh];
                for (size_t i = 0; i < mesh.primitives.size(); i++) {
                    const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
                    uint32_t firstIndex = static_cast<uint32_t>(indices.size());
                    uint32_t vertexStart = static_cast<uint32_t>(vertices.size());
                    uint32_t indexCount = 0;
                    // Vertices
				    {
                    const float* positionBuffer = nullptr;
                    const float* normalsBuffer = nullptr;
                    const float* texCoordsBuffer = nullptr;
                    size_t vertexCount = 0;

                        // Get buffer data for vertex normals
                        if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                            const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                            const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                            positionBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                            vertexCount = accessor.count;
                        }
                        // Get buffer data for vertex normals
                        if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                            const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                            const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                            normalsBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        }
                        // Get buffer data for vertex texture coordinates
                        // glTF supports multiple sets, we only load the first one
                        if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                            const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                            const tinygltf::BufferView& view = glTFInput.bufferViews[accessor.bufferView];
                            texCoordsBuffer = reinterpret_cast<const float*>(&(glTFInput.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        }

                        					// Append data to model's vertex buffer
                        for (size_t v = 0; v < vertexCount; v++) {
                            Vertex vert{};
                            vert.position = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                            vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                            vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                            vert.color = glm::vec3(1.0f);
                            vertices.push_back(vert);
                        }
                    }
                    // Indices
                    {
                        const tinygltf::Accessor& accessor = glTFInput.accessors[glTFPrimitive.indices];
                        const tinygltf::BufferView& bufferView = glTFInput.bufferViews[accessor.bufferView];
                        const tinygltf::Buffer& buffer = glTFInput.buffers[bufferView.buffer];

                        indexCount += static_cast<uint32_t>(accessor.count);

                        // glTF supports different component types of indices
                        switch (accessor.componentType) {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                            const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                            const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                            const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                            for (size_t index = 0; index < accessor.count; index++) {
                                indices.push_back(buf[index] + vertexStart);
                            }
                            break;
                        }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                            return;
                        }
                    }
                    // Primitive primitive{};
                    // primitive.firstIndex = firstIndex;
                    // primitive.indexCount = indexCount;
                    // primitive.materialIndex = glTFPrimitive.material;
                    // node.mesh.primitives.push_back(primitive);
                }
            }
        }
    }

        
}

}
