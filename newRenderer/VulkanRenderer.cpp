/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include "VulkanDisplay.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>


VulkanRenderer2::VulkanRenderer2(RendererBase& base, VkSurfaceKHR& surface, uint32_t index, uint32_t width, uint32_t height) : rendererBase(base), gpu(base.physicalDevices[index])
{
	combinedQueueFamily = FindCombinedQueueFamily(surface);
	std::array<float, 1> queuePriorities = {0.0F};
	VkDeviceQueueCreateInfo queueCI;
	queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCI.pNext = nullptr;
	queueCI.flags = 0;
	queueCI.queueFamilyIndex = combinedQueueFamily;
	queueCI.queueCount = 1;
	queueCI.pQueuePriorities = queuePriorities.data();

	VkDeviceCreateInfo deviceCI;
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = nullptr;
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &queueCI;
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
	deviceCI.ppEnabledExtensionNames = deviceExtensionNames.data();
	deviceCI.pEnabledFeatures = nullptr;

	VkResult result = vkCreateDevice(gpu.physicalDevice, &deviceCI, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan device\n");
	}

	vkGetDeviceQueue(device, combinedQueueFamily, 0, &combinedQueue);

	VkCommandPoolCreateInfo commandPoolCI;
	commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCI.pNext = nullptr;
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCI.queueFamilyIndex = combinedQueueFamily;

	result = vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool for combined queue family\n");
	}

	VkCommandBufferAllocateInfo commandBufferAI;
	commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAI.pNext = nullptr;
	commandBufferAI.commandPool = commandPool;
	commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAI.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer from combined queue family command pool\n");
	}

	VkSemaphoreCreateInfo semaphoreCI;
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;

	result = vkCreateSemaphore(device, &semaphoreCI, nullptr, &imageAvailableSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}
	result = vkCreateSemaphore(device, &semaphoreCI, nullptr, &renderFinishedSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}

	VkFenceCreateInfo fenceCI;
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	result = vkCreateFence(device, &fenceCI, nullptr, &inFlightFence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fence\n");
	}

	display = std::make_unique<VulkanDisplay>(this, surface, width, height);
}

VulkanRenderer2::~VulkanRenderer2()
{
	//delete display;
	vkDeviceWaitIdle(device);
	display.reset();

	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

uint32_t VulkanRenderer2::FindCombinedQueueFamily(VkSurfaceKHR& surface)
{
	for (uint32_t i = 0; i < gpu.queueFamilyProperties.size(); i++)
	{
		VkBool32 isSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physicalDevice, i, surface, &isSupported);
		if ((gpu.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && isSupported)
		{
			return i;
		}
	}
	throw std::runtime_error("No combined queue family found\n");
}

void VulkanRenderer2::Render()
{
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);


	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, display.get()->swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkImage& image = display.get()->image.get()->images[imageIndex];

	// Eventually command buffer recording should be done in a RendererDescription or Drawable object
	vkResetCommandBuffer(commandBuffer, 0);
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	// Move the image into the correct layout
	// No? just use clearcolorimage
	VkImageMemoryBarrier generalToClearBarrier;
	generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	generalToClearBarrier.pNext = nullptr;
	generalToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	generalToClearBarrier.image = image;
	generalToClearBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
	//VK_PIPELINE_STAGE_TRANSFER_BIT
	// Use this for now to clear the image to a specific color
	VkClearColorValue clearColor = {0.42F, 1.0F, 0.46F, 1.0F};

	vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);

	VkImageMemoryBarrier clearToPresentBarrier;
	clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	clearToPresentBarrier.pNext = nullptr;
	clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.image = image;
	clearToPresentBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);


	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer
	std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore};
	std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
	std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	VkResult result = vkQueueSubmit(combinedQueue, 1, &submitInfo, inFlightFence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
	}

	std::array<VkSwapchainKHR, 1> swapchains = {display.get()->swapchain};
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = signalSemaphores.size();
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = swapchains.size();
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(combinedQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present image\n");
	}
}

void VulkanRenderer2::Resize(VkSurfaceKHR surface, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
{
	vkDeviceWaitIdle(device);
	display.reset();

	display = std::make_unique<VulkanDisplay>(this, surface, width, height);
}
















