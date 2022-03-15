#pragma once

#include <memory>
#include <string>

namespace hyd
{

template <typename T>
class IManager
{
public:
    virtual bool loadRessource(const std::string& id) = 0;
    virtual std::shared_ptr<T> getRessource(const std::string& id) = 0;
    virtual std::shared_ptr<T> loadAndGetRessource(const std::string& id) = 0;
};

} // namespace hyd
