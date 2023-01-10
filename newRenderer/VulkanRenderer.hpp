/*
 * VulkanRenderer.hpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#ifndef NEWRENDERER_VULKANRENDERER_HPP_
#define NEWRENDERER_VULKANRENDERER_HPP_

#include <vulkan/vulkan.h>
#include <vector>

struct PhysicalDeviceDescriptor
{
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
};

class VulkanRenderer2
{
public:
	explicit VulkanRenderer2(const char* const applicationName);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT flags,
			VkDebugUtilsMessageTypeFlagsEXT objType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* userData);
	//void enumeratePhysicalDevices;
	VkInstance instance;
	std::vector<PhysicalDeviceDescriptor> physicalDevices;
	VkDebugUtilsMessengerEXT debugMessenger;
	bool enableValidationLayers = true;
private:
	VkResult setupDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();
};


#endif /* NEWRENDERER_VULKANRENDERER_HPP_ */
