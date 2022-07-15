/*
 * VulkanSwapChain.hpp
 *
 *  Created on: Apr 11, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSWAPCHAIN_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSWAPCHAIN_HPP_

#include "Header.hpp"
#include <QWindow>
#include <QVulkanInstance>
class VulkanInstance;
class VulkanDevice;
class VulkanRenderer;
class VulkanApplication;

struct SwapChainBuffer
{
	VkImage image;
	VkImageView view;
};

struct SwapChainPrivateVariables
{
	// Store the image surface capabilities
	VkSurfaceCapabilitiesKHR surfCapabilities;

	// Store the number of present modes suppoted by the implementation
	uint32_t presentModeCount;

	// Arrays for retrieved present modes
	std::vector<VkPresentModeKHR> presentModes;

	// Size of the swap chain color images
	VkExtent2D swapChainExtent;

	// Number of color images supported by the implementation
	uint32_t desiredNumberOfSwapChainImages;

	VkSurfaceTransformFlagBitsKHR preTransform;

	// Stores present mode bitwise flag for the creation of the swap chain
	VkPresentModeKHR swapchainPresentMode;

	// The retrieved drawing color swap chain images
	std::vector<VkImage> swapchainImages;

	std::vector<VkSurfaceFormatKHR> surfFormats;
};

struct SwapChainPublicVariables
{
	// The logical platform dependent surface object
	VkSurfaceKHR surface;

	// Number of buffer images used for swap chain
	uint32_t swapchainImageCount;

	// Swap chain object
	VkSwapchainKHR swapChain;

	// List of color swap chain images
	std::vector<SwapChainBuffer> colorBuffer;

	// Semaphore for sync purpose
	VkSemaphore presentCompleteSemaphore;

	// Current drawing surface index in use
	uint32_t currentColorBuffer;

	// Format of the image
	VkFormat format;
};

class VulkanSwapChain
{
public:
	VulkanSwapChain(VulkanRenderer* renderer, QVulkanInstance& qVkInstance, QWindow* qWindow);
	~VulkanSwapChain();
	void initializeSwapChain();
	void createSwapChain(const VkCommandBuffer& cmd);
	void destroySwapChain();

	// User defined structure containing private variables used by the swapchain private and public functions
	SwapChainPublicVariables scPublicVars;

	PFN_vkQueuePresentKHR fpQueuePresentKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;

	void setSwapChainExtent(uint32_t swapChainWidth, uint32_t swapChainHeight);

private:
	VkResult createSwapChainExtensions();
	void getSupportedFormats();
	VkResult createSurface();
	uint32_t getGraphicsQueueWithPresentationSupport();
	void getSurfaceCapabilitiesAndPresentMode();
	void managePresentMode();
	void createSwapChainColorImages();
	void createColorImageView(const VkCommandBuffer& cmd);


	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkDestroySurfaceKHR fpDestroySurfaceKHR;

	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;

	// User defined structure containing private variables used by the swapchain private and public functions
	SwapChainPrivateVariables scPrivateVars;
	VulkanRenderer* rendererObj;
	VulkanApplication* appObj;

	QVulkanInstance& m_qVkInstance;
	QWindow* m_qWindow;
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSWAPCHAIN_HPP_ */
