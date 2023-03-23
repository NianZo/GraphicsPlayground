/*
 * VulkanRenderer.hpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#ifndef RENDERER_VULKANRENDERER_HPP_
#define RENDERER_VULKANRENDERER_HPP_

#include "RendererBase.hpp"
#include "VulkanDisplay.hpp"
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
// class VulkanDisplay;

// struct PhysicalDeviceDescriptor
//{
//	VkPhysicalDevice physicalDevice;
//	VkPhysicalDeviceFeatures features;
//	VkPhysicalDeviceProperties properties;
//	VkPhysicalDeviceMemoryProperties memoryProperties;
//	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
// };

class VulkanRenderer
{
  public:
    VulkanRenderer(RendererBase& base, VkSurfaceKHR& surface, uint32_t index, uint32_t width, uint32_t height);
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&&) noexcept;
    VulkanRenderer& operator=(VulkanRenderer&&) = delete; // Can't do this because rendererBase can't be changed
    ~VulkanRenderer();

    uint32_t FindCombinedQueueFamily(VkSurfaceKHR& surface);

    void Render();
    void Resize(VkSurfaceKHR surface, uint32_t width, uint32_t height);

    RendererBase& rendererBase;
    std::unique_ptr<VulkanDisplay> display;
    PhysicalDeviceDescriptor& gpu;
    VkDevice device;
    uint32_t combinedQueueFamily;
    VkQueue combinedQueue;

    VkCommandPool commandPool;

  private:
    std::vector<const char*> deviceExtensionNames =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

#endif /* RENDERER_VULKANRENDERER_HPP_ */
