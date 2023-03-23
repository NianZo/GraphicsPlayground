/*
 * RendererCore.hpp
 *
 *  Created on: Jan 30, 2023
 *      Author: nic
 */

#ifndef RENDERER_RENDERERBASE_HPP_
#define RENDERER_RENDERERBASE_HPP_

#include <filesystem>
#include <vector>
#include <vulkan/vulkan.h>

struct __attribute__((aligned(128))) PhysicalDeviceDescriptor
{
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
};

class RendererBase
{
  public:
    explicit RendererBase(std::filesystem::path projectDirectory, const char* applicationName);
    RendererBase(const RendererBase&) = delete; // TODO(nic) I could actually implement these by just creating a new object with same parameters
    RendererBase& operator=(const RendererBase&) = delete;
    RendererBase(RendererBase&&) noexcept;
    RendererBase& operator=(RendererBase&&) noexcept;
    ~RendererBase();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* userData);

    VkInstance instance;
    std::vector<PhysicalDeviceDescriptor> physicalDevices;
    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableValidationLayers = true;
    std::filesystem::path projectDirectory;

  private:
    VkResult setupDebugMessenger();
    static VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();
};

#endif /* RENDERER_RENDERERBASE_HPP_ */
