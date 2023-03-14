/*
 * RendererCore.cpp
 *
 *  Created on: Jan 30, 2023
 *      Author: nic
 */

#include "RendererBase.hpp"

#include <iostream>

RendererBase::RendererBase(const char* const applicationName)
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
	// Indexing issue if this is const auto&
	for (const VkPhysicalDevice& physicalDevice : physicalDeviceList)
	{
		PhysicalDeviceDescriptor descriptor;
		descriptor.physicalDevice = physicalDevice;
		vkGetPhysicalDeviceFeatures(physicalDevice, &descriptor.features);
		vkGetPhysicalDeviceProperties(physicalDevice, &descriptor.properties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &descriptor.memoryProperties);
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		descriptor.queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, descriptor.queueFamilyProperties.data());
		physicalDevices.emplace_back(descriptor);
	}

	if (enableValidationLayers)
	{
		res = setupDebugMessenger();
		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger");
		}
	}

}

RendererBase::~RendererBase()
{
	if (enableValidationLayers)
	{
		auto vkDestroyDebugUtilsMessengerEXT =
				reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (vkDestroyDebugUtilsMessengerEXT == nullptr)
		{
			std::cerr << "Failed to load function vkDestroyDebugUtilsMessengerEXT in ~RendererBase()\n";
		}
		vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(instance, nullptr);
}

VkDebugUtilsMessengerCreateInfoEXT RendererBase::populateDebugMessengerCreateInfo()
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

VkResult RendererBase::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	auto vkCreateDebugUtilsMessengerEXT =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (vkCreateDebugUtilsMessengerEXT == nullptr)
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	return vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
}

VKAPI_ATTR VkBool32 VKAPI_CALL RendererBase::debugCallback(
		[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		[[maybe_unused]] void* userData)
{
	std::cerr << "[VK_DEBUG_REPORT] " << pCallbackData->pMessage << std::endl;
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		//throw std::runtime_error("Encountered validation message of severity higher than warning. Treating as runtime error\n");
	}

	return VK_TRUE;
}
