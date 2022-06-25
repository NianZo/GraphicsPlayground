/*
 * VulkanDevice.cpp
 *
 *  Created on: Mar 30, 2022
 *      Author: nic
 */

#include "VulkanDevice.hpp"
#include <iostream>

VulkanDevice::VulkanDevice(VkPhysicalDevice* physicalDevice)
{
	gpu = physicalDevice;
}

VkResult VulkanDevice::createDevice(std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	VkResult result;
	float queuePriorities[1] = {0.0F};
	assert(extensions.size() != 0);
	std::cout << "device extensions[0]: " << extensions[0] << std::endl;

	VkDeviceQueueCreateInfo queueInfo;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	vkGetPhysicalDeviceFeatures(*gpu, &deviceFeatures);
	VkPhysicalDeviceFeatures enabledFeatures = {VK_FALSE};
	enabledFeatures.samplerAnisotropy = deviceFeatures.samplerAnisotropy;

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledLayerCount = 0; // Device layers are deprecated
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.enabledExtensionCount = extensions.size();
	deviceInfo.ppEnabledExtensionNames = extensions.data();
	deviceInfo.pEnabledFeatures = &enabledFeatures;

	result = vkCreateDevice(*gpu, &deviceInfo, nullptr, &device);

	assert(result == VK_SUCCESS);
	std::cout << "finished createDevice in VulkanDevice" << std::endl;
	return result;
}

void VulkanDevice::getPhysicalDeviceQueuesAndProperties()
{
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, nullptr);
	queueFamilyProps.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueFamilyProps.data());
}

void VulkanDevice::getGraphicsQueueHandle()
{
	bool found = false;

	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			graphicsQueueFamilyIndex = i;
			break;
		}
	}

	return;
}

// Deferred until we talk about presentation and swapchains
void VulkanDevice::getDeviceQueue()
{
	vkGetDeviceQueue(device, graphicsQueueWithPresentIndex, 0, &queue);
}

bool VulkanDevice::memoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, uint32_t* typeIndex)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

