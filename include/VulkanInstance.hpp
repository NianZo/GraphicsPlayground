/*
 * VulkanInstance.hpp
 *
 *  Created on: Mar 21, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANINSTANCE_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANINSTANCE_HPP_

#include "Header.hpp"
#include "VulkanLayerAndExtension.hpp"

class VulkanInstance
{
public:
	// Many lines skipped
	// TODO fill me in

	// Vulkan instance object
	VkInstance instance;

	// Vulkan instance specific layer and extensions
	VulkanLayerAndExtension layerExtension;

	// Functions for creation and deletion of Vulkan Instance
	VkResult createInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* const applicationName);
	void destroyInstance();
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANINSTANCE_HPP_ */
