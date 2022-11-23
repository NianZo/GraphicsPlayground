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

	//void enumeratePhysicalDevices;

	VkInstance instance;
	std::vector<PhysicalDeviceDescriptor> physicalDevices;

};


#endif /* NEWRENDERER_VULKANRENDERER_HPP_ */
