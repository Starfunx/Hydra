#pragma once

#include "iManager.hpp"
#include "Managers/TextureManager.hpp"

#include "Renderer/Material.hpp"
#include "Renderer/DescriptorSet.hpp"
#include "Renderer/Device.hpp"

// std
#include <unordered_map>
#include <memory>

namespace hyd
{
    
class MaterialManager : IManager<Material>
{
public:
    MaterialManager(Device& device, 
        DescriptorLayoutCache& descriptorLayoutCache,
        DescriptorAllocator& descriptorAllocator,
        TextureManager& textureManager);
    ~MaterialManager();

    /*for now a material is a single texture. (and its descriptor)*/
    bool loadRessource(const std::string& id);
    std::shared_ptr<Material> getRessource(const std::string& id);
    std::shared_ptr<Material> loadAndGetRessource(const std::string& id);

private:
    /* data */
    Device& m_device;
    DescriptorLayoutCache& m_descriptorLayoutCache;
    DescriptorAllocator& m_descriptorAllocator;
    TextureManager& m_textureManager;

    std::unordered_map<std::string, std::shared_ptr<Material>> m_ressources;
};

} // namespace hyd
