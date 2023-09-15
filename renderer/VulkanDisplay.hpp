/*
 * VulkanDisplay.hpp
 *
 *  Created on: Jan 19, 2023
 *      Author: nic
 */

#ifndef RENDERER_VULKANDISPLAY_HPP_
#define RENDERER_VULKANDISPLAY_HPP_

#include "Camera.hpp"
#include "VulkanImage.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanRenderer;

class VulkanDisplay
{
  public:
    VulkanDisplay(VulkanRenderer& renderer, VkSurfaceKHR& surface, uint32_t width, uint32_t height);
    VulkanDisplay(const VulkanDisplay&) = delete;
    VulkanDisplay& operator=(const VulkanDisplay&) = delete;
    VulkanDisplay(VulkanDisplay&&) noexcept;
    VulkanDisplay& operator=(VulkanDisplay&&) = delete;
    ~VulkanDisplay();

    VulkanRenderer& renderer;
    VkSurfaceKHR surface;
    // uint32_t queueFamilyIndex;

    VkSwapchainKHR swapchain;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    // Arrays for retrieved present modes
    std::vector<VkPresentModeKHR> presentModes;

    // Size of the swap chain color images
    VkExtent2D swapchainExtent;

    VkSurfaceTransformFlagBitsKHR preTransform;

    // Stores present mode bitwise flag for the creation of the swap chain
    VkPresentModeKHR swapchainPresentMode;

    VkFormat swapchainImageFormat;

    // The retrieved drawing color swap chain images
    // std::vector<VkImage> swapchainImages;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    //std::unique_ptr<VulkanImage> image;
    //Camera& camera;

  private:
    // VkPhysicalDevice& gpu;
    // VkSurfaceKHR& surface;
};

#endif /* RENDERER_VULKANDISPLAY_HPP_ */
