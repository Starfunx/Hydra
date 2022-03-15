#include "TextureManager.hpp"

namespace hyd
{

TextureManager::TextureManager(Device& device)
: m_device{device}
{}

TextureManager::~TextureManager(){

}


bool TextureManager::loadRessource(const std::string& id){
    std::shared_ptr<Texture> newRessource = std::make_shared<Texture>(m_device, id);
    m_ressources[id] = newRessource;
    return true;
}

std::shared_ptr<Texture> TextureManager::getRessource(const std::string& id){
    return m_ressources[id];
}

std::shared_ptr<Texture> TextureManager::loadAndGetRessource(const std::string& id){
    loadRessource(id);
    return getRessource(id);
}



} // namespace hyd
