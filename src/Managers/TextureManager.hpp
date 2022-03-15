#pragma once

#include "iManager.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/Device.hpp"

// std
#include <unordered_map>
#include <memory>

namespace hyd
{
    
class TextureManager : IManager<Texture>
{
public:
    TextureManager(Device& device);
    ~TextureManager();

    bool loadRessource(const std::string& id);
    std::shared_ptr<Texture> getRessource(const std::string& id);
    std::shared_ptr<Texture> loadAndGetRessource(const std::string& id);

private:
    /* data */
    Device& m_device;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_ressources;
};

} // namespace hyd
