/*
 * VulkanDisplay.cpp
 *
 *  Created on: Jan 21, 2023
 *      Author: nic
 */

#include "VulkanDisplay.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanImage.hpp"
#include "algorithm"
#include <limits>

#include <iostream>

VulkanDisplay::VulkanDisplay(VulkanRenderer2* renderer, VkSurfaceKHR& displaySurface, uint32_t width, uint32_t height)
{
	m_renderer = renderer;
	surface = displaySurface;
	m_width = width;
	m_height = height;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderer->gpu.physicalDevice, surface, &surfaceCapabilities);

	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->gpu.physicalDevice, surface, &surfaceFormatCount, nullptr);
	surfaceFormats.resize(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderer->gpu.physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

	uint32_t surfacePresentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderer->gpu.physicalDevice, surface, &surfacePresentModesCount, nullptr);
	presentModes.resize(surfacePresentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderer->gpu.physicalDevice, surface, &surfacePresentModesCount, presentModes.data());

	preTransform = surfaceCapabilities.currentTransform;
	swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
//	if (surfaceCapabilities.minImageExtent.width > width || surfaceCapabilities.minImageExtent.height > height)
//	{
//		swapchainExtent = surfaceCapabilities.minImageExtent;
//	}
//	else
//	{
//		swapchainExtent = {width, height};
//	}
	swapchainExtent.width = std::clamp(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
	swapchainExtent.height = std::clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	std::cout << "swapchainExtent: " << width << ", " << height << "\n";
	std::cout << "minImageExtent: " << surfaceCapabilities.minImageExtent.width << ", " << surfaceCapabilities.minImageExtent.height << "\n";
	std::cout << "maxImageExtent: " << surfaceCapabilities.maxImageExtent.width << ", " << surfaceCapabilities.maxImageExtent.height << "\n";
	//VkExtent2D(width, height, 1);

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = swapchainImageFormat;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent = swapchainExtent;//surfaceCapabilities.minImageExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	vkCreateSwapchainKHR(m_renderer->device, &createInfo, nullptr, &swapchain);

	image = std::make_unique<VulkanImage>(*this);
//	fpGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
//	fpAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
//	fpQueuePresentKHR = (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
}

VulkanDisplay::~VulkanDisplay()
{
	std::cout << "Starting ~VulkanDisplay()\n";
	image.reset();
	vkDestroySwapchainKHR(m_renderer->device, swapchain, nullptr);
}


