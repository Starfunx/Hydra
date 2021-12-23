#pragma once

#include "Device.hpp"

// std
#include <string>
#include <vector>

namespace hyd
{

struct PipelineConfigInfo
{
    /* data */
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
};


class Pipeline
{
public:
    Pipeline(
        Device& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo);

    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline operator=(const Pipeline&) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

private:
    static std::vector<char> readFile(const std::string& filepath);
    void createGraphicspipeline(
        const std::string& vertFilepath, 
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo);

    void createShaderModule(
        const std::vector<char>& code,
        VkShaderModule* shaderModule);
    /* data */
    Device& m_device;
    VkPipeline m_graphicsPipeline;
    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShadermodule;
};


} // namespace se
