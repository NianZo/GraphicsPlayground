/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include <stdexcept>
#include <vector>


VulkanRenderer2::VulkanRenderer2(const char* const applicationName)
{
	std::vector<const char*> layers;
	std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xcb_surface"}; // TODO this is an issue and will come back to haunt me on cross-platform support

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = applicationName;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = applicationName;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	// Define the Vulkan instance create info structure
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;//&layerExtension.dbgReportCreateInfo;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instanceInfo.ppEnabledLayerNames = layers.size() ? layers.data() : nullptr;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.size() ? extensions.data() : nullptr;
	VkResult res = vkCreateInstance(&instanceInfo, nullptr, &instance);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create VkInstance\n");
	}
	//assert(res == VK_SUCCESS);
	//return res;
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDeviceList(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDeviceList.data());
	physicalDevices.resize(physicalDeviceCount);
	for (const auto& physicalDevice : physicalDeviceList)
	{
		PhysicalDeviceDescriptor descriptor;
		descriptor.physicalDevice = physicalDevice;
		vkGetPhysicalDeviceFeatures(physicalDevice, &descriptor.features);
		vkGetPhysicalDeviceProperties(physicalDevice, &descriptor.properties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &descriptor.memoryProperties);
		physicalDevices.emplace_back(descriptor);
	}
}
