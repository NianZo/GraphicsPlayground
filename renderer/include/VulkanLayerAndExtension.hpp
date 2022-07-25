/*
 * VulkanLayerAndExtension.hpp
 *
 *  Created on: Mar 20, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANLAYERANDEXTENSION_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANLAYERANDEXTENSION_HPP_

#include <vector>
#include "Header.hpp"

struct LayerProperties
{
	VkLayerProperties properties;
	std::vector<VkExtensionProperties> extensions;
};

class VulkanLayerAndExtension
{
public:
	VulkanLayerAndExtension() {fillDbgReportCreateInfo();};
	std::vector<const char*> instanceExtensionNames;
	std::vector<const char*> instanceLayerNames;
	// Layers and corresponding extension list
	std::vector<LayerProperties> layerPropertyList;
	VkResult getInstanceLayerProperties();

	// Global extensions
	VkResult getExtensionProperties(LayerProperties& layerProps, VkPhysicalDevice* gpu = nullptr);

	// Device based extensions
	VkResult getDeviceExtensionProperties(VkPhysicalDevice* gpu);

	VkBool32 areLayersSupported(std::vector<const char*>& layerNames);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugFunction(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* layerPrefix, const char* msg, void* userData);

	PFN_vkCreateDebugReportCallbackEXT dbgCreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT dbgDestroyDebugReportCallback;
	VkDebugReportCallbackEXT debugReportCallback;
	VkResult createDebugReportCallback();
	void destroyDebugReportCallback();
	VkDebugReportCallbackCreateInfoEXT dbgReportCreateInfo = {};
	void fillDbgReportCreateInfo();
};



#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANLAYERANDEXTENSION_HPP_ */
