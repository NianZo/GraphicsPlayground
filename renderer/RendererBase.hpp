/*
 * RendererCore.hpp
 *
 *  Created on: Jan 30, 2023
 *      Author: nic
 */

#ifndef RENDERER_RENDERERBASE_HPP_
#define RENDERER_RENDERERBASE_HPP_

#include <vector>
#include <vulkan/vulkan.h>

struct PhysicalDeviceDescriptor
{
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
};

class RendererBase
{
public:
	RendererBase(const char* const applicationName);
	~RendererBase();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT flags,
			VkDebugUtilsMessageTypeFlagsEXT objType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* userData);

	VkInstance instance;
	std::vector<PhysicalDeviceDescriptor> physicalDevices;
	VkDebugUtilsMessengerEXT debugMessenger;
	bool enableValidationLayers = true;

private:
	VkResult setupDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();
};


#endif /* RENDERER_RENDERERBASE_HPP_ */
