/*
 * VulkanSwapChain.cpp
 *
 *  Created on: Apr 11, 2022
 *      Author: nic
 */

#include "VulkanSwapChain.hpp"

#include "VulkanDevice.hpp"
#include "VulkanInstance.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanApplication.hpp"
#include <iostream>
#include <xcb/xcb.h>


VulkanSwapChain::VulkanSwapChain(VulkanRenderer* renderer)
{
	rendererObj = renderer;
	appObj = VulkanApplication::GetInstance();
}

VulkanSwapChain::~VulkanSwapChain()
{
	scPrivateVars.swapchainImages.clear();
	scPrivateVars.surfFormats.clear();
	scPrivateVars.presentModes.clear();
}

void VulkanSwapChain::initializeSwapChain()
{
	VkResult result;
	createSwapChainExtensions();
	result = createSurface();
	if (result == VK_ERROR_INITIALIZATION_FAILED) std::cout << "window surface initialization failed\n";
	if (result == VK_ERROR_EXTENSION_NOT_PRESENT) std::cout << "window surface extension not present\n";
	if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) std::cout << "window surface in use\n";
	assert(result == VK_SUCCESS);

	uint32_t index = getGraphicsQueueWithPresentationSupport();
	if (index == UINT32_MAX)
	{
		std::cout << "Could not find a graphics and present queue\nCould not find a graphics and present queue\n";
		exit(-1);
	}
	rendererObj->getDevice()->graphicsQueueWithPresentIndex = index;

	getSupportedFormats();
	std::cout << "finished initializeSwapChain" << std::endl;
}

void VulkanSwapChain::createSwapChain(const VkCommandBuffer& cmd)
{
	std::cout << "starting creatingSwapChain" << std::endl;
	getSurfaceCapabilitiesAndPresentMode();
	std::cout << "finished getSurfaceCapabilitiesAndPresentMode" << std::endl;
	managePresentMode();
	std::cout << "finished managePresentMode" << std::endl;
	createSwapChainColorImages();
	std::cout << "finished createSwapChainColorImages" << std::endl;
	createColorImageView(cmd);
	std::cout << "finished creatingSwapChain" << std::endl;
}

VkResult VulkanSwapChain::createSwapChainExtensions()
{
	// Dependency on createPresentationWindow()
	VkInstance& instance = appObj->instanceObj.instance;
	VkDevice& device = appObj->deviceObj->device;

	// Get instance based swapchain extension function pointers
	// TODO I'm not convinced these all need to be dynamically queried
	fpGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	fpGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	fpGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	fpGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	fpDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");

	// Get device based swapchain extension function pointers
	// TODO '''
	fpCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
	fpDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
	fpGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
	fpAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
	fpQueuePresentKHR = (PFN_vkQueuePresentKHR) vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
	std::cout << "finished createSwapChainExtensions" << std::endl;
	return VK_SUCCESS;
}

VkResult VulkanSwapChain::createSurface()
{
	VkInstance& instance = appObj->instanceObj.instance;

	return glfwCreateWindowSurface(instance, rendererObj->window, nullptr, &scPublicVars.surface);

	//VkXcbSurfaceCreateInfoKHR
	VkXcbSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	int scr;

	createInfo.connection = xcb_connect(NULL, &scr);
	if (info.connection == NULL || xcb_connection_has_error(createInfo.connection)) {
	    std::cout << "Unable to make an XCB connection\n";
	    exit(-1);
	}

	setup = xcb_get_setup(createInfo.connection);
	iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0) xcb_screen_next(&iter);

	//info.screen = iter.data;

	//createInfo.connection = ;
	//createInfo.window = window;
}

uint32_t VulkanSwapChain::getGraphicsQueueWithPresentationSupport()
{
	VulkanDevice* device = appObj->deviceObj;
	uint32_t queueCount = device->queueFamilyCount;
	VkPhysicalDevice gpu = *device->gpu;
	std::vector<VkQueueFamilyProperties>& queueProps = device->queueFamilyProps;

	// Iterate each queue family and get presentation status for each
	std::vector<VkBool32> supportsPresent;
	supportsPresent.resize(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
	{
		supportsPresent[i] = VK_FALSE;
		VkResult result = fpGetPhysicalDeviceSurfaceSupportKHR(gpu, i, scPublicVars.surface, &supportsPresent[i]);
		assert(result == VK_SUCCESS);
	}

	// Search for a graphics queue that supports presentation
	// TODO this algorithm makes no sense... both paths look for the same thing
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i ++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			graphicsQueueNodeIndex = i;
		}
		if (supportsPresent[i] == VK_TRUE)
		{
			graphicsQueueNodeIndex = i;
			presentQueueNodeIndex = i;
			std::cout << "found combined graphics+present queue. Index: " << i << std::endl;
			break;
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		std::cout << "didn't find a queue that supports both graphics and present... this isn't good, device needs to be created with both of these" << std::endl;
		// If didn't find a queue that supports both graphics and present, then find a separate present queue
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find queue with present capabilities
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
	{
		return UINT32_MAX;
	}
	return graphicsQueueNodeIndex;
}

void VulkanSwapChain::getSupportedFormats()
{
	VkPhysicalDevice gpu = *rendererObj->getDevice()->gpu;
	VkResult result;
	// Get number of formats supported
	uint32_t formatCount;
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, nullptr);
	assert(result == VK_SUCCESS);
	scPrivateVars.surfFormats.clear();
	scPrivateVars.surfFormats.resize(formatCount);

	// Get formats in allocated objects
	result = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, scPublicVars.surface, &formatCount, scPrivateVars.surfFormats.data());
	assert(result == VK_SUCCESS);
	// If we just get VK_FORMAT_UNDEFINED then there is no preferred format; use RGBA32 format
	if (formatCount == 1 && scPrivateVars.surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		scPublicVars.format = VK_FORMAT_B8G8R8A8_UNORM;
	} else
	{
		scPublicVars.format = scPrivateVars.surfFormats[0].format;
	}
}

void VulkanSwapChain::getSurfaceCapabilitiesAndPresentMode()
{
	VkResult result;
	VkPhysicalDevice gpu = *appObj->deviceObj->gpu;
	std::cout << "getSurfaceCapabilitiesAndPresentMode 1" << std::endl;
	result = fpGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, scPublicVars.surface, &scPrivateVars.surfCapabilities);
	assert(result == VK_SUCCESS);
	std::cout << "getSurfaceCapabilitiesAndPresentMode 2" << std::endl;
	result = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, scPublicVars.surface, &scPrivateVars.presentModeCount, nullptr);
	assert(result == VK_SUCCESS);
	std::cout << "getSurfaceCapabilitiesAndPresentMode 3" << std::endl;
	scPrivateVars.presentModes.clear();
	scPrivateVars.presentModes.resize(scPrivateVars.presentModeCount);
	std::cout << "getSurfaceCapabilitiesAndPresentMode 4" << std::endl;
	result = fpGetPhysicalDeviceSurfacePresentModesKHR(gpu, scPublicVars.surface, &scPrivateVars.presentModeCount, scPrivateVars.presentModes.data());
	assert(result == VK_SUCCESS);
	std::cout << "getSurfaceCapabilitiesAndPresentMode 5" << std::endl;
	if (scPrivateVars.surfCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If surface width and height aren't defined then set equal to image size
		scPrivateVars.swapChainExtent.width = rendererObj->width;
		scPrivateVars.swapChainExtent.height = rendererObj->height;
	} else
	{
		scPrivateVars.swapChainExtent = scPrivateVars.surfCapabilities.currentExtent;
	}
}

void VulkanSwapChain::managePresentMode()
{
	// Prefere MAILBOX - lowest latency non-tearing mode
	// Then prefer IMMEDIATE - fastest (but tears)
	// Then fall back to FIFO
	scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (size_t i = 0; i < scPrivateVars.presentModeCount; i++)
	{
		if (scPrivateVars.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if (scPrivateVars.swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR && scPrivateVars.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			scPrivateVars.swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine number of VkImages to use in swapchain
	scPrivateVars.desiredNumberOfSwapChainImages = scPrivateVars.surfCapabilities.minImageCount + 1;
	if ((scPrivateVars.surfCapabilities.maxImageCount > 0) && (scPrivateVars.desiredNumberOfSwapChainImages > scPrivateVars.surfCapabilities.maxImageCount))
	{
		// Settle for fewer images than desired
		scPrivateVars.desiredNumberOfSwapChainImages = scPrivateVars.surfCapabilities.maxImageCount;
	}

	if (scPrivateVars.surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		scPrivateVars.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else
	{
		scPrivateVars.preTransform = scPrivateVars.surfCapabilities.currentTransform;
	}
}

void VulkanSwapChain::createSwapChainColorImages()
{
	VkSwapchainCreateInfoKHR scInfo;
	scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scInfo.pNext = nullptr;
	scInfo.flags = 0; // TODO file bug on Vulkan validation layers (or submit pull request), vkCreateSwapchainKHR segfaults if this isn't 0 and validation layers don't catch it
	scInfo.surface = scPublicVars.surface;
	scInfo.minImageCount = scPrivateVars.desiredNumberOfSwapChainImages;
	scInfo.imageFormat = scPublicVars.format;
	scInfo.imageExtent.width = scPrivateVars.swapChainExtent.width;
	scInfo.imageExtent.height = scPrivateVars.swapChainExtent.height;
	scInfo.preTransform = scPrivateVars.preTransform;
	scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	scInfo.imageArrayLayers = 1;
	scInfo.presentMode = scPrivateVars.swapchainPresentMode;
	scInfo.oldSwapchain = VK_NULL_HANDLE;
	scInfo.clipped = true;
	scInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	scInfo.queueFamilyIndexCount = 0;
	scInfo.pQueueFamilyIndices = nullptr;
	VkDevice& testDevice = rendererObj->getDevice()->device;
	std::cout << "got rendererObj->getDevice()->device" << std::endl;
	assert(fpCreateSwapchainKHR != nullptr);
	VkResult result = fpCreateSwapchainKHR(rendererObj->getDevice()->device, &scInfo, nullptr, &scPublicVars.swapChain);
	assert(result == VK_SUCCESS);
	std::cout << "finished fpCreateSwapchainKHR" << std::endl;
	fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, nullptr);
	scPrivateVars.swapchainImages.clear();
	scPrivateVars.swapchainImages.resize(scPublicVars.swapchainImageCount);
	fpGetSwapchainImagesKHR(rendererObj->getDevice()->device, scPublicVars.swapChain, &scPublicVars.swapchainImageCount, scPrivateVars.swapchainImages.data());
}

void VulkanSwapChain::createColorImageView(const VkCommandBuffer& cmd)
{
	VkResult result;
	scPublicVars.colorBuffer.clear();
	for (uint32_t i = 0; i < scPublicVars.swapchainImageCount; i++)
	{
		SwapChainBuffer scBuffer;

		VkImageViewCreateInfo imgViewInfo;
		imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imgViewInfo.pNext = nullptr;
		imgViewInfo.format = scPublicVars.format;
		imgViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imgViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imgViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imgViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgViewInfo.subresourceRange.baseMipLevel = 0;
		imgViewInfo.subresourceRange.levelCount = 1;
		imgViewInfo.subresourceRange.baseArrayLayer = 0;
		imgViewInfo.subresourceRange.layerCount = 1;
		imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imgViewInfo.flags = 0;

		scBuffer.image = scPrivateVars.swapchainImages[i];
		// Since the swapchain is not owned by use we cannot set the image layout.
		// Upon setting, the implementation (driver) may give error, the images were created by the WSI implementation not by us.
		imgViewInfo.image = scBuffer.image;
		result = vkCreateImageView(rendererObj->getDevice()->device, &imgViewInfo, nullptr, &scBuffer.view);
		scPublicVars.colorBuffer.push_back(scBuffer);
	}
	scPublicVars.currentColorBuffer = 0;
}

void VulkanSwapChain::setSwapChainExtent(uint32_t swapChainWidth, uint32_t swapChainHeight)
{
	scPrivateVars.swapChainExtent.width = swapChainWidth;
	scPrivateVars.swapChainExtent.height = swapChainHeight;
}

void VulkanSwapChain::destroySwapChain()
{
	VulkanDevice* deviceObj = appObj->deviceObj;

	for (uint32_t i = 0; i < scPublicVars.swapchainImageCount; i++)
	{
		vkDestroyImageView(deviceObj->device, scPublicVars.colorBuffer[i].view, nullptr);
	}

	if (!appObj->isResizing)
	{
		// Only executes at application shutdown
		std::cout << "Destroying swapchain and surface" << std::endl;
		fpDestroySwapchainKHR(deviceObj->device, scPublicVars.swapChain, nullptr);
		vkDestroySurfaceKHR(appObj->instanceObj.instance, scPublicVars.surface, nullptr);
	}
}























