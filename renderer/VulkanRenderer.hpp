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
    ~VulkanRenderer();

    uint32_t FindCombinedQueueFamily(VkSurfaceKHR& surface);

    void Render();
    void Resize(VkSurfaceKHR surface, uint32_t width, uint32_t height);
    //	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    //			VkDebugUtilsMessageSeverityFlagBitsEXT flags,
    //			VkDebugUtilsMessageTypeFlagsEXT objType,
    //			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    //			void* userData);
    // void chooseGPU(VkSurfaceKHR& surface, uint32_t index);
    // void enumeratePhysicalDevices;
    //	VkInstance instance;
    //	std::vector<PhysicalDeviceDescriptor> physicalDevices;
    //	VkDebugUtilsMessengerEXT debugMessenger;
    //	bool enableValidationLayers = true;
    RendererBase& rendererBase;
    std::unique_ptr<VulkanDisplay> display;
    PhysicalDeviceDescriptor& gpu;
    // uint32_t activeGPU;
    VkDevice device;
    uint32_t combinedQueueFamily;
    VkQueue combinedQueue;

    VkCommandPool commandPool;
    // TODO probably shouldn't live here
    VkCommandBuffer commandBuffer;

    // VkSemaphore imageAvailableSemaphore;
    // VkSemaphore renderFinishedSemaphore;
    // VkFence inFlightFence;
  private:
    std::vector<const char*> deviceExtensionNames =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    static std::vector<char> readFile(const std::string& filename);
    // VkSurfaceKHR surface;
    //	VkResult setupDebugMessenger();
    //	VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();
};

#endif /* RENDERER_VULKANRENDERER_HPP_ */
