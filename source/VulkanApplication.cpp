/*
 * VulkanApplication.cpp
 *
 *  Created on: Mar 26, 2022
 *      Author: nic
 */

#include "VulkanApplication.hpp"
#include <iostream>

std::vector<const char*> instanceExtensionNames =
{
		VK_KHR_SURFACE_EXTENSION_NAME,
		"VK_KHR_xcb_surface",
		//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

//std::vector<const char*> layerNames =
//{
//		//"VK_LAYER_LUNARG_api_dump",
//		"VK_LAYER_GOOGLE_threading",
//		"VK_LAYER_LUNARG_parameter_validation",
//		"VK_LAYER_LUNARG_device_limits",
//		"VK_LAYER_LUNARG_object_tracker",
//		"VK_LAYER_LUNARG_image",
//		"VK_LAYER_LUNARG_core_validation",
//		"VK_LAYER_LUNARG_swapchain",
//		"VK_LAYER_GOOGLE_unique_objects"
//};

std::vector<const char *> layerNames = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> deviceExtensionNames =
{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<VkPhysicalDevice> gpuList;





// Returns the Singleton object of VulkanApplication
VulkanApplication* VulkanApplication::GetInstance()
{
	//static std::unique_ptr<VulkanApplication> instance = std::make_unique<VulkanApplication>();
	static VulkanApplication instance;
	return &instance;
}

VulkanApplication::VulkanApplication()
{
	// At application start up, enumerate instance layers
	instanceObj.layerExtension.getInstanceLayerProperties();

	deviceObj = nullptr;

	isResizing = false;
	isPrepared = false;
	deviceObj = nullptr;
	rendererObj = nullptr;

	char title[] = "Hello World!";

	// Check if the supplied layers are supported or not
	instanceObj.layerExtension.areLayersSupported(layerNames);

	// Create the Vulkan instance with
	// specified layer and extension names.
	std::cout << "Start initializing VulkanApplication" << std::endl;
	createVulkanInstance(layerNames, instanceExtensionNames, title);
}

VulkanApplication::~VulkanApplication()
{
	delete rendererObj;
	rendererObj = nullptr;
}

VkResult VulkanApplication::createVulkanInstance(std::vector<const char*>& layers, std::vector<const char*>& extensions, const char* appName)
{
	instanceObj.createInstance(layers, extensions, appName);
	return VK_SUCCESS;
}

void VulkanApplication::initialize(QVulkanInstance& qVkInstance, QWindow* qWindow, uint32_t width, uint32_t height)
{


	// Get list of PhysicalDevices (gpus) in the system
	std::cout << "About to enumerate physical devices..." << std::endl;
	std::vector<VkPhysicalDevice> gpuList;
	enumeratePhysicalDevices(gpuList);
	// Use the first GPU found for this exercise
	if (gpuList.size() > 0)
	{
		std::cout << "About to handshake with device" <<std::endl;
		handShakeWithDevice(&gpuList[0], layerNames, deviceExtensionNames);
	}

	if (!rendererObj)
	{
		std::cout << "About to create vulkan renderer\n";
		rendererObj = new VulkanRenderer(this, deviceObj, qVkInstance, qWindow);
		std::cout << "Just created vulkan renderer\n";
		rendererObj->createPresentationWindow(width, height);
		rendererObj->getSwapChain()->initializeSwapChain();
	}

	rendererObj->initialize();
}

void VulkanApplication::prepare()
{
	isPrepared = false;
	rendererObj->prepare();
	isPrepared = true;
}

bool VulkanApplication::render()
{
	if (!isPrepared) return false;
	return rendererObj->render();
}

VkResult VulkanApplication::enumeratePhysicalDevices(std::vector<VkPhysicalDevice>& gpuList)
{
	uint32_t gpuDeviceCount;
	std::cout << "about to use instanceObj in enumeratePhysicalDevices" << std::endl;
	vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, nullptr);
	std::cout << "Used instanceObj in enumeratePhysicalDevices" << std::endl;
	gpuList.resize(gpuDeviceCount);
	return vkEnumeratePhysicalDevices(instanceObj.instance, &gpuDeviceCount, gpuList.data());
}

VkResult VulkanApplication::handShakeWithDevice(VkPhysicalDevice* gpu, std::vector<const char*>& layers, std::vector<const char*>& extensions)
{
	// Our abstraction of Vulkan logical and physical device
	// Manages queues and their properties
	deviceObj = new VulkanDevice(gpu);
	if (deviceObj == nullptr)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	// Print the devices available, layers, and extensions
	deviceObj->layerExtension.getDeviceExtensionProperties(gpu);

	// Get the physical device (GPU) properties
	vkGetPhysicalDeviceProperties(*gpu, &deviceObj->gpuProps);

	// Get memory properties from the physical device or GPU
	vkGetPhysicalDeviceMemoryProperties(*gpu, &deviceObj->memoryProperties);

	// Query queues and properties from GPU
	deviceObj->getPhysicalDeviceQueuesAndProperties();

	// Retrieve the queue which supports graphics pipeline
	deviceObj->getGraphicsQueueHandle();

	// Create logical device; ensure that the device can use the graphics queue
	deviceObj->createDevice(layers, extensions);

	return VK_SUCCESS;
}

void VulkanApplication::resize()
{
	if (!isPrepared)
	{
		std::cout << "resizing window... but application was not prepared first" << std::endl;
		return;
	}

	isResizing = true;
	std::cout << "Resizing window... waiting for device to go idle" << std::endl;
	vkDeviceWaitIdle(deviceObj->device);
	rendererObj->destroyFramebuffers();
	rendererObj->destroyCommandPool();
	rendererObj->destroyPipeline();
	rendererObj->getPipelineObject()->destroyPipelineCache();
	rendererObj->destroyRenderpass();
	rendererObj->getSwapChain()->destroySwapChain();
	rendererObj->destroyDrawableVertexBuffer();
	rendererObj->destroyDepthBuffer();
	std::cout << "Destroyed objects, re-initializing" << std::endl;
	rendererObj->initialize();
	prepare();

	isResizing = false;
}

void VulkanApplication::update()
{
	rendererObj->update();
}








