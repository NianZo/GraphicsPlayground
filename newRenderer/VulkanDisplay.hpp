/*
 * VulkanDisplay.hpp
 *
 *  Created on: Jan 19, 2023
 *      Author: nic
 */

#ifndef NEWRENDERER_VULKANDISPLAY_HPP_
#define NEWRENDERER_VULKANDISPLAY_HPP_

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "VulkanImage.hpp"

class VulkanRenderer2;

class VulkanDisplay
{
public:
	VulkanDisplay(VulkanRenderer2* renderer, VkSurfaceKHR& surface, uint32_t width, uint32_t height);
	~VulkanDisplay();

	uint32_t m_width;
	uint32_t m_height;
	//uint32_t queueFamilyIndex;
	VkSurfaceKHR surface;
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
	//std::vector<VkImage> swapchainImages;

	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::unique_ptr<VulkanImage> image;
	VulkanRenderer2* m_renderer;
private:
	//VkPhysicalDevice& gpu;
	//VkSurfaceKHR& surface;

};


#endif /* NEWRENDERER_VULKANDISPLAY_HPP_ */
