/*
 * RendererTestBasic.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"

#include "VulkanRenderer.hpp"

#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//extern int g_argc;
extern char** g_argv;

TEST(RenderTestBasic, CreateRendererBase)
{
	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(g_argv[0])).parent_path();
	RendererBase rendererBase(pwd, "RenderTestBasic");
	ASSERT_NE(rendererBase.physicalDevices.size(), 0);

	// Verify that the structures describing the physical devices are filled out for at least one valid GPU
	// Ignore GPUs with vendorID 0 since it can't be used
	bool validGPUFound = false;
	for (const PhysicalDeviceDescriptor& gpuDescriptor : rendererBase.physicalDevices)
	{
		if (gpuDescriptor.properties.vendorID != 0)
		{
			validGPUFound = true;
			EXPECT_EQ(gpuDescriptor.features.geometryShader, VK_TRUE);
			EXPECT_NE(gpuDescriptor.properties.apiVersion, 0);
			EXPECT_NE(gpuDescriptor.memoryProperties.memoryTypeCount, 0);
			EXPECT_NE(gpuDescriptor.memoryProperties.memoryHeapCount, 0);
		}
	}
	EXPECT_TRUE(validGPUFound);
}

TEST(RenderTestBasic, CreateRenderer)
{
	{
		std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(g_argv[0])).parent_path();
		RendererBase rendererBase(pwd, "RenderTestBasic");
		ASSERT_NE(rendererBase.physicalDevices.size(), 0);

		auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
		ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());
		//auto gpuDescriptor = getValidPhysicalDevice(rendererBase);
		//ASSERT_TRUE(gpuDescriptor);

	    VulkanRenderer renderer(rendererBase, *gpuDescriptor);
	}
}

TEST(RenderTestBasic, RendererWithSurface)
{
	{
		std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(g_argv[0])).parent_path();
		RendererBase rendererBase(pwd, "RenderTestBasic");
		ASSERT_NE(rendererBase.physicalDevices.size(), 0);

		auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
		ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());

	    glfwInit();
	    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
		VkSurfaceKHR surface;
	    if (glfwCreateWindowSurface(rendererBase.instance, window, nullptr, &surface) != VK_SUCCESS)
	    {
	        throw std::runtime_error("failed to create window surface!");
	    }

	    {
	    	VulkanRenderer renderer(rendererBase, surface, *gpuDescriptor, 800, 600);
	    }

	    vkDestroySurfaceKHR(rendererBase.instance, surface, nullptr);
	}
}

//TEST(RenderTestBasic, BasicCamera)
//{
//	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(g_argv[0])).parent_path();
//	RendererBase rendererBase(pwd, "RenderTestBasic");
//	ASSERT_NE(rendererBase.physicalDevices.size(), 0);
//
//	uint32_t gpuIndex = 1;
//	VulkanRenderer renderer(rendererBase, rendererBase.physicalDevices[gpuIndex]);
//
//	renderer.scenes.emplace_back(renderer);
//	renderer.scenes[0].cameras.emplace_back(renderer, 800, 600);
//
//	renderer.scenes[0].cameras[0].clear();
//	//EXPECT_FALSE(renderer.scenes[0].cameras[0].cpuDataAvailable);
//	ImageData<800, 600>& cameraData = renderer.scenes[0].cameras[0].cpuData();
//
//	EXPECT_EQ(cameraData[0][0], 0);
//	EXPECT_EQ(cmaeraData[0][599], 0);
//	EXPECT_EQ(cameraData[799][0], 0);
//	EXPECT_EQ(cameraData[799][599], 0);
//	// Alternatively
//	EXPECT_EQ(cameraData.front().front(), 0);
//	EXPECT_EQ(cameraData.front().back(), 0);
//	EXPECT_EQ(cameraData.back().front(), 0);
//	EXPECT_EQ(cameraData.back().back(), 0);
//}
