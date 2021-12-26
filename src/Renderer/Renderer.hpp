#pragma once

#include "Core/Window.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace hyd
{
    
class Renderer
{
public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer &operator=(const Renderer&) = delete;

    VkRenderPass getSwapChainRenderPass() { return m_swapChain->getRenderPass(); }
    bool isFrameInProgress() const { return m_isFrameStarted;}

    VkCommandBuffer getCurrentCommandBuffer() const { 
        assert(m_isFrameStarted && "Cannot get command buffer when no frame is in progress");
        return m_commandBuffers[m_currentImageIndex];
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    void setFrameBufferResized() { m_frameBufferResized = true; }

private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    /* data */
    Window& m_window;
    Device& m_device;
    std::unique_ptr<SwapChain> m_swapChain;
    
    std::vector<VkCommandBuffer> m_commandBuffers;

    uint32_t m_currentImageIndex;
    bool m_isFrameStarted;

    bool m_frameBufferResized{false};
};

} // namespace se

