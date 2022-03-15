#include "MeshManager.hpp"

namespace hyd
{

MeshManager::MeshManager(Device& device)
: m_device{device}
{}

MeshManager::~MeshManager(){

}


bool MeshManager::loadRessource(const std::string& id){
    std::shared_ptr<Model> newRessource = Model::createModelFromFile(m_device, id);
    m_ressources[id] = newRessource;
    return true;
}

std::shared_ptr<Model> MeshManager::getRessource(const std::string& id){
    return m_ressources[id];
}

std::shared_ptr<Model> MeshManager::loadAndGetRessource(const std::string& id){
    loadRessource(id);
    return getRessource(id);
}



} // namespace hyd
