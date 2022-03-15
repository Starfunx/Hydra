#include "MaterialManager.hpp"

namespace hyd
{

MaterialManager::MaterialManager(
    Device& device, 
    DescriptorLayoutCache& descriptorLayoutCache,
    DescriptorAllocator& descriptorAllocator,
    TextureManager& textureManager
) : m_device{device}, 
    m_descriptorLayoutCache{descriptorLayoutCache}, 
    m_descriptorAllocator{descriptorAllocator}, 
    m_textureManager{textureManager}
{}

MaterialManager::~MaterialManager(){

}


bool MaterialManager::loadRessource(const std::string& id){
    std::shared_ptr<Material> newRessource = std::make_shared<Material>();
    newRessource->m_textures = std::vector<std::shared_ptr<Texture>>({m_textureManager.getRessource(id)});

    auto ptexture = m_textureManager.getRessource(id);
    auto imageInfo = ptexture->getImageInfo();
    DescriptorBuilder(m_descriptorLayoutCache, m_descriptorAllocator)
        .bind_image(0, &imageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build(newRessource->m_descriptor);

    m_ressources[id] = newRessource;
    return true;
}

std::shared_ptr<Material> MaterialManager::getRessource(const std::string& id){
    return m_ressources[id];
}

std::shared_ptr<Material> MaterialManager::loadAndGetRessource(const std::string& id){
    loadRessource(id);
    return getRessource(id);
}



} // namespace hyd
