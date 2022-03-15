#pragma once

#include "iManager.hpp"
#include "Renderer/Model.hpp"
#include "Renderer/Device.hpp"

// std
#include <unordered_map>
#include <memory>

namespace hyd
{
    
class MeshManager : IManager<Model>
{
public:
    MeshManager(Device& device);
    ~MeshManager();

    bool loadRessource(const std::string& id);
    std::shared_ptr<Model> getRessource(const std::string& id);
    std::shared_ptr<Model> loadAndGetRessource(const std::string& id);

private:
    /* data */
    Device& m_device;

    std::unordered_map<std::string, std::shared_ptr<Model>> m_ressources;
};

} // namespace hyd
