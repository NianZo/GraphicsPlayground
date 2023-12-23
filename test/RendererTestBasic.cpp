/*
 * RendererTestBasic.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"

#include "VulkanRenderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//extern int g_argc;
extern char** g_argv;

TEST(RenderTestBasic, CreateRenderer)
{
	{
		std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(g_argv[0])).parent_path();
		RendererBase rendererBase(pwd, "RenderTestBasic");
		//VulkanRenderer2 renderer("RenderTestBasic");
		EXPECT_NE(rendererBase.physicalDevices.size(), 0);

		// Verify that the structures describing the physical devices are filled out for at least one valid gpu
		// Can't verify for all because I have a 'null'? gpu with vendor id 0 and no features
		bool validGPUFound = false;
		uint32_t gpuIndex = 1;
		for (const PhysicalDeviceDescriptor& gpuDescriptor : rendererBase.physicalDevices)
		{
			if (gpuDescriptor.properties.vendorID != 0)
			{
				validGPUFound = true;
				//gpuIndex =
				EXPECT_EQ(gpuDescriptor.features.geometryShader, VK_TRUE);
				EXPECT_NE(gpuDescriptor.properties.apiVersion, 0);
				EXPECT_NE(gpuDescriptor.memoryProperties.memoryTypeCount, 0);
				EXPECT_NE(gpuDescriptor.memoryProperties.memoryHeapCount, 0);
			}
		}
		EXPECT_TRUE(validGPUFound);

	    glfwInit();
	    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
		VkSurfaceKHR surface;
	    if (glfwCreateWindowSurface(rendererBase.instance, window, nullptr, &surface) != VK_SUCCESS) {
	        throw std::runtime_error("failed to create window surface!");
	    }

	    {
	    	VulkanRenderer renderer(rendererBase, surface, gpuIndex, 800, 600);
	    }
	    vkDestroySurfaceKHR(rendererBase.instance, surface, nullptr);
	}
}
