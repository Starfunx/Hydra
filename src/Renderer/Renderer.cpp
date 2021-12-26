#include "Renderer.hpp"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace hyd
{

Renderer::Renderer(Window& window, Device& device):
    m_window{window}, m_device{device}{
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer(){
    freeCommandBuffers();
}

void Renderer::recreateSwapChain(){
    
    auto extent{m_window.getExtent()};
    while (extent.width == 0 || extent.height == 0) {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_device.device());

    if (m_swapChain == nullptr){
        m_swapChain = std::make_unique<SwapChain>(m_device, extent);
    } else {
        m_swapChain = std::make_unique<SwapChain>(m_device, extent, std::move(m_swapChain));
        if (m_swapChain->imageCount() != m_commandBuffers.size()){
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    // come back
}


void Renderer::createCommandBuffers(){
    m_commandBuffers.resize(m_swapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers(){
    vkFreeCommandBuffers(
        m_device.device(),
        m_device.getCommandPool(),
        static_cast<uint32_t>( m_commandBuffers.size()),
        m_commandBuffers.data());
    m_commandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame(){
    assert(!m_isFrameStarted && "Can't call beginFrame while already in progress!");
    
    auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error(" failed to acquire next swap chain image!");
    }

    m_isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("failed to begin recording command buffer");
    }

    return commandBuffer;
}

void Renderer::endFrame(){
    assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress!");
    auto commanfBuffer = getCurrentCommandBuffer();

    if (vkEndCommandBuffer(commanfBuffer) != VK_SUCCESS){
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = m_swapChain->submitCommandBuffers(&commanfBuffer, &m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || 
            m_frameBufferResized) {
        m_frameBufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS){
        throw std::runtime_error("failed to present swapchain image!");
    }

    m_isFrameStarted = false;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
    assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if freame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && 
        "Can't call beginSwapChainRenderPass if freame is not in progress!");
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain->getRenderPass();
    renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.5, 0.2, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
    assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress!");
    assert(commandBuffer == getCurrentCommandBuffer() && 
        "Can't end beginSwapChainRenderPass if freame is not in progress!");
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace se
