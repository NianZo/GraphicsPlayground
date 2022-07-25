/*
 * VulkanInstance.cpp
 *
 *  Created on: Mar 21, 2022
 *      Author: nic
 */

#include "VulkanInstance.hpp"
#include <iostream>

VkResult VulkanInstance::createInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* const applicationName)
{
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	// TODO, this is a big hack!
	//glfwInit();
	std::cout << "abotu to query extensions\n";
	//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // why does this not work and returns zero extensions?
	std::cout << "queryied extensions:\n" << glfwExtensionCount << std::endl;
	for (int i = 0; i < glfwExtensionCount; i++)
	{
		std::cout << glfwExtensions[i] << std::endl;
	}
	//printf("%s\n", glfwExtensions[0]);

	// Set the instance specific layer and extension information
	layerExtension.instanceExtensionNames = extensions;
	layerExtension.instanceLayerNames = layers;

	// Define the Vulkan application structure
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
	instanceInfo.pNext = &layerExtension.dbgReportCreateInfo;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = (uint32_t) layers.size();
	instanceInfo.ppEnabledLayerNames = layers.size() ? layers.data() : nullptr;
	instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.size() ? extensions.data() : nullptr;
	VkResult res = vkCreateInstance(&instanceInfo, nullptr, &instance);
	assert(res == VK_SUCCESS);
	return res;
}

void VulkanInstance::destroyInstance()
{
	vkDestroyInstance(instance, nullptr);
}

