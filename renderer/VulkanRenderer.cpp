/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include "Drawable.hpp"
#include "VulkanDisplay.hpp"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

VulkanRenderer::VulkanRenderer(RendererBase& base, VkSurfaceKHR& surface, uint32_t index, uint32_t width, uint32_t height) :
		rendererBase(base),
		gpu(base.physicalDevices[index]),
		device(VK_NULL_HANDLE),
		combinedQueueFamily(FindCombinedQueueFamily(surface)),
		combinedQueue(VK_NULL_HANDLE),
		commandPool(VK_NULL_HANDLE)
{
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

    display = std::make_unique<VulkanDisplay>(*this, surface, width, height);
}

VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) noexcept :
		rendererBase(other.rendererBase),
		display(std::move(other.display)),
		gpu(other.gpu),
		device(other.device),
		combinedQueueFamily(other.combinedQueueFamily),
		combinedQueue(other.combinedQueue),
		commandPool(other.commandPool)

{
	other.device = VK_NULL_HANDLE;
	other.combinedQueue = VK_NULL_HANDLE;
	other.commandPool = VK_NULL_HANDLE;
}

VulkanRenderer::~VulkanRenderer()
{
    // delete display;
    vkDeviceWaitIdle(device);
    display.reset();

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
}

uint32_t VulkanRenderer::FindCombinedQueueFamily(VkSurfaceKHR& surface)
{
    for (uint32_t i = 0; i < gpu.queueFamilyProperties.size(); i++)
    {
        VkBool32 isSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physicalDevice, i, surface, &isSupported);
        if (((gpu.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U) && isSupported != 0U)
        {
            return i;
        }
    }
    throw std::runtime_error("No combined queue family found\n");
}

void VulkanRenderer::Render()
{
    Drawable drawable(*this, commandPool);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(device, display->swapchain, UINT64_MAX, drawable.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    // VkImage& image = display.get()->image.get()->images[imageIndex];
    // drawable.ClearWindow(image);

    drawable.RenderTriangle(imageIndex);

    drawable.ExecuteCommandBuffer();

    std::array<VkSemaphore, 1> signalSemaphores = {drawable.renderFinishedSemaphore};

    std::array<VkSwapchainKHR, 1> swapchains = {display->swapchain};
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = signalSemaphores.size();
    presentInfo.pWaitSemaphores = signalSemaphores.data();
    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    const VkResult result = vkQueuePresentKHR(combinedQueue, &presentInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present image\n");
    }
}

void VulkanRenderer::Resize(VkSurfaceKHR surface, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
{
    vkDeviceWaitIdle(device);
    display.reset();

    display = std::make_unique<VulkanDisplay>(*this, surface, width, height);
}


