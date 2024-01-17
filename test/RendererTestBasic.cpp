/*
 * RendererTestBasic.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"

#include "VulkanRenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <optional>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
// Example of using stb_image to write out the rendered image. When run from eclipse this is put in home directory
//const int ret = stbi_write_png("screenshot.png", 800, 600, 4, cameraData.data.data(), 800 * 4);

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


extern std::vector<char*> args;

TEST(RenderTestBasic, CreateRendererBase)
{
	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
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
		std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
		RendererBase rendererBase(pwd, "RenderTestBasic");
		ASSERT_NE(rendererBase.physicalDevices.size(), 0);

		auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
		ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());

	    VulkanRenderer renderer(rendererBase, *gpuDescriptor);
	}
}

TEST(RenderTestBasic, RendererWithSurface)
{
	{
		std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
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

TEST(RenderTestBasic, BasicCamera)
{
	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
	RendererBase rendererBase(pwd, "RenderTestBasic");
	ASSERT_NE(rendererBase.physicalDevices.size(), 0);

	auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
	ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());

    VulkanRenderer renderer(rendererBase, *gpuDescriptor);

	renderer.scenes.emplace_back(renderer, 800, 600);
	renderer.scenes[0].clearColor = {.uint32 = {42, 1, 2, 3}};
	renderer.scenes[0].render();
	ImageData& cameraData = renderer.scenes[0].renderTargetCpuData();

	EXPECT_EQ(cameraData.index(0, 0).r, 42);
	EXPECT_EQ(cameraData.index(0, 599).r, 42);
	EXPECT_EQ(cameraData.index(799, 0).r, 42);
	EXPECT_EQ(cameraData.index(799, 599).r, 42);
}

TEST(RenderTestBasic, BasicDrawable)
{
	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
	RendererBase rendererBase(pwd, "RenderTestBasic");
	ASSERT_NE(rendererBase.physicalDevices.size(), 0);

	auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
	ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());

    VulkanRenderer renderer(rendererBase, *gpuDescriptor);

	renderer.scenes.emplace_back(renderer, 800, 600);

	const std::filesystem::path shaderDirectory = renderer.rendererBase.projectDirectory.parent_path() / "shaders";
    GraphicsPipelineDescriptor descriptor;
    descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
    descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};

    // Fullscreen quad
	const std::vector<Vertex> vertices = {
			{{-1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{1.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{-1.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}}
	};
	const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
	};

	descriptor.vertexData = vertices;
	descriptor.indexData = indices;
	renderer.scenes[0].drawables.emplace_back(renderer.scenes[0], descriptor);

	renderer.scenes[0].clearColor = {.uint32 = {42, 1, 2, 255}};
	renderer.scenes[0].render();
	ImageData& cameraData = renderer.scenes[0].renderTargetCpuData();

	EXPECT_EQ(cameraData.index(0, 0).r, 255);
	EXPECT_EQ(cameraData.index(0, 599).r, 255);
	EXPECT_EQ(cameraData.index(799, 0).r, 255);
	EXPECT_EQ(cameraData.index(799, 599).r, 255);
	EXPECT_EQ(cameraData.index(400, 300).r, 255);
}

TEST(RenderTestBasic, DrawableWithTransform)
{
	std::filesystem::path pwd = std::filesystem::weakly_canonical(std::filesystem::path(args[0])).parent_path();
	RendererBase rendererBase(pwd, "RenderTestBasic");
	ASSERT_NE(rendererBase.physicalDevices.size(), 0);

	auto gpuDescriptor = std::ranges::find_if(rendererBase.physicalDevices, [](PhysicalDeviceDescriptor& pdd){return pdd.properties.vendorID != 0;});
	ASSERT_NE(gpuDescriptor, rendererBase.physicalDevices.end());

    VulkanRenderer renderer(rendererBase, *gpuDescriptor);

	renderer.scenes.emplace_back(renderer, 800, 600);

	const std::filesystem::path shaderDirectory = renderer.rendererBase.projectDirectory.parent_path() / "shaders";
    GraphicsPipelineDescriptor descriptor;
    descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
    descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};

    // Fullscreen quad
	const std::vector<Vertex> vertices = {
			{{-1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{1.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
			{{-1.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}}
	};
	const std::vector<uint16_t> indices = {
			0, 1, 2, 2, 3, 0
	};

	descriptor.vertexData = vertices;
	descriptor.indexData = indices;
	renderer.scenes[0].drawables.emplace_back(renderer.scenes[0], descriptor);

	renderer.scenes[0].drawables[0].transform = glm::translate(glm::mat4(1.0F), glm::vec3(-1.0F, 0.0F, 0.0F));

	renderer.scenes[0].clearColor = {.uint32 = {42, 1, 2, 255}};
	renderer.scenes[0].render();
	ImageData& cameraData = renderer.scenes[0].renderTargetCpuData();

	EXPECT_EQ(cameraData.index(0, 0).r, 255);
	EXPECT_EQ(cameraData.index(0, 599).r, 255);
	EXPECT_EQ(cameraData.index(799, 0).r, 42);
	EXPECT_EQ(cameraData.index(799, 599).r, 42);
	EXPECT_EQ(cameraData.index(399, 300).r, 255);
	EXPECT_EQ(cameraData.index(400, 300).r, 42);
}
