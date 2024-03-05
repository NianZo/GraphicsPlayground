/*
 * VulkanRenderer.hpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#ifndef RENDERER_VULKANRENDERER_HPP_
#define RENDERER_VULKANRENDERER_HPP_

#include "Camera.hpp"
#include "Drawable.hpp"
#include "RendererBase.hpp"
#include "Scene.hpp"
#include "VulkanDisplay.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <vulkan/vulkan.h>
// class VulkanDisplay;
class Camera;
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
    VulkanRenderer(RendererBase& base, VkSurfaceKHR& surface, PhysicalDeviceDescriptor& physicalDevice, uint32_t width, uint32_t height);
    VulkanRenderer(RendererBase& base, PhysicalDeviceDescriptor& physicalDevice);
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&&) noexcept;
    VulkanRenderer& operator=(VulkanRenderer&&) = delete; // Can't do this because rendererBase can't be changed
    ~VulkanRenderer();

    uint32_t FindCombinedQueueFamily(VkSurfaceKHR& surface);
    uint32_t FindCombinedQueueFamily();

    void Render();
    // void Render(const GraphicsPipelineDescriptor& pipelineDescriptor);
    void Resize(VkSurfaceKHR surface, uint32_t width, uint32_t height);

    RendererBase& rendererBase;
    std::unique_ptr<VulkanDisplay> display;
    PhysicalDeviceDescriptor& gpu;
    VkDevice device;
    uint32_t combinedQueueFamily;
    VkQueue combinedQueue;

    VkCommandPool commandPool;

    std::vector<Scene> scenes;

    std::jthread renderThread;
    std::mutex renderMutex;
    // std::vector<Drawable> drawables;
    // std::vector<Camera> cameras;
    // Camera camera;

  private:
    std::vector<const char*> deviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // static void renderLoop(std::stop_token stopToken, VulkanRenderer& renderer);
};

#endif /* RENDERER_VULKANRENDERER_HPP_ */
