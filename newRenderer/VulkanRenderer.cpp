/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>


VulkanRenderer2::VulkanRenderer2(const char* const applicationName)
{
	std::vector<const char*> layers;
	std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_xcb_surface"}; // TODO this is an issue and will come back to haunt me on cross-platform support
	if (enableValidationLayers)
	{
		// TODO check for validation layer availability here
		layers.emplace_back("VK_LAYER_KHRONOS_validation");
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = populateDebugMessengerCreateInfo();

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
	instanceInfo.pNext = enableValidationLayers ? &debugCreateInfo : nullptr;
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

	res = setupDebugMessenger();
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger");
	}
}

VkDebugUtilsMessengerCreateInfoEXT VulkanRenderer2::populateDebugMessengerCreateInfo()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.messageSeverity =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	return createInfo;
}

VkResult VulkanRenderer2::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (vkCreateDebugUtilsMessengerEXT == nullptr)
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	return vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer2::debugCallback(
		[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		[[maybe_unused]] void* userData)
{
	std::cerr << "[VK_DEBUG_REPORT] " << pCallbackData->pMessage << std::endl;
	return VK_TRUE;
}





















