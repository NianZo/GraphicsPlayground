/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanDisplay.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>
#include <fstream>


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

	display = std::make_unique<VulkanDisplay>(this, surface, width, height);
}

VulkanRenderer2::~VulkanRenderer2()
{
	//delete display;
	vkDeviceWaitIdle(device);
	display.reset();

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
	Drawable drawable(*this, commandPool);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, display.get()->swapchain, UINT64_MAX, drawable.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	//VkImage& image = display.get()->image.get()->images[imageIndex];
	//drawable.ClearWindow(image);

	drawable.RenderTriangle(imageIndex);

	drawable.ExecuteCommandBuffer();

	std::array<VkSemaphore, 1> signalSemaphores = {drawable.renderFinishedSemaphore};

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

	VkResult result = vkQueuePresentKHR(combinedQueue, &presentInfo);
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

std::vector<char> VulkanRenderer2::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open shader file: " + filename + "\n");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
	file.close();

	return buffer;
}














