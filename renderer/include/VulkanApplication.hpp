/*
 * VulkanApplication.hpp
 *
 *  Created on: Mar 26, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANAPPLICATION_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANAPPLICATION_HPP_

#include "VulkanInstance.hpp"
#include "VulkanLayerAndExtension.hpp"
#include "VulkanDevice.hpp"
#include "VulkanRenderer.hpp"
#include <mutex>


class VulkanApplication
{
public:
	~VulkanApplication();

	static VulkanApplication* GetInstance();

	// Program life cycle
	void initialize(VkSurfaceKHR* surface, uint32_t width, uint32_t height); // Initialize and allocate resources
	void prepare(); // Prepare resources
	void update(); // Update data
	bool render(); // Render primitives
	void deInitialize(); // Release resources
	void resize(); // Resize the presentation surface

	VulkanInstance instanceObj;
	VulkanDevice* deviceObj;
	VulkanRenderer* rendererObj;
	//static std::unique_ptr<VulkanApplication> instance;
	bool isPrepared;
	bool isResizing;
	bool shouldClose;

private:
	VulkanApplication(); // Private to enforce Singleton pattern


	static std::once_flag onlyOnce;


	VkResult createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* applicationName);
	VkResult enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpuList);
	VkResult handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions);
};



#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANAPPLICATION_HPP_ */
