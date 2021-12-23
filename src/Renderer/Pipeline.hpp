#pragma once

#include <string>
#include <vector>

namespace hyd
{
class Pipeline
{
public:
    Pipeline(const std::string& vertFilepath, const std::string& fragFilepath);

private:
    static std::vector<char> readFile(const std::string& filepath);
    void createGraphicspipeline(const std::string& vertFilepath, 
        const std::string& fragFilepath);
    /* data */
};


} // namespace se
