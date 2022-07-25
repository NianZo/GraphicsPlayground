/*
 * VulkanRenderer.cpp
 *
 *  Created on: Apr 6, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include "VulkanApplication.hpp"
#include "MeshData.hpp"
#include <iostream>
//#include <vulkan/vulkan_core.h>

//void onWindowResized(GLFWwindow* window, int newWidth, int newHeight);

VulkanRenderer::VulkanRenderer(VulkanApplication* app, VulkanDevice* deviceObject, VkSurfaceKHR* surface)
{
	assert(app != nullptr);
	assert(deviceObject != nullptr);

	// Note: very important to initialize with 0 or it will break the system
	memset(&Depth, 0, sizeof(Depth));
	//memset(&connection, 0, sizeof(HINSTANCE));

	application = app;
	deviceObj = deviceObject;

	swapChainObj = new VulkanSwapChain(this, surface);
	VulkanDrawable* drawableObj = new VulkanDrawable(this);
	drawableList.push_back(drawableObj);
}

VulkanRenderer::~VulkanRenderer()
{
	delete swapChainObj;
	swapChainObj = nullptr;
}

void VulkanRenderer::createPresentationWindow(const int windowWidth, const int windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	// Do a linux thing here :)
	//glfwInit(); // TODO, this instance needs to query glfw for extensions, so it calls this first
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//window = glfwCreateWindow(windowWidth, windowHeight, "Learning Vulkan Window", nullptr, nullptr);

	// For window resizing
	//glfwSetWindowUserPointer(window, this);
	//glfwSetWindowSizeCallback(window, onWindowResized);
}

void onWindowResized(int newWidth, int newHeight)
{
	std::cout << "onWindowResized starting..." << std::endl;
	VulkanApplication* appObj = VulkanApplication::GetInstance();
	VulkanRenderer* rendererObj = appObj->rendererObj;
	rendererObj->width = newWidth;
	rendererObj->height = newHeight;
	rendererObj->getSwapChain()->setSwapChainExtent(newWidth, newHeight);
	appObj->resize();
}

void VulkanRenderer::initialize()
{
	// Create an empty window with dimension 500x500
//	createPresentationWindow(500, 500);
//	width = 500;
//	height = 500;
//	// Initialize swapchain
//	swapChainObj->initializeSwapChain();

	// Need command buffers, so create a command buffer pool
	createCommandPool();
	// Let's create the swapchain color images
	buildSwapChainAndDepthImage();
	createVertexBuffer();
	createIndexBuffer();
	const bool includeDepth = true;
	createRenderPass(includeDepth);
	createFramebuffers(includeDepth);

	createShaders();
	std::cout << "created shaders" << std::endl;


	const char* filename = "/home/nic/Documents/Projects/GraphicsPlayground/texture.jpg";
	bool renderOptimalTexture = true;
	if (renderOptimalTexture)
	{
		createTextureOptimal(filename, &texture, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
	} else
	{
		createTextureLinear(filename, &texture, VK_IMAGE_USAGE_SAMPLED_BIT);
	}

	createDescriptors();
	std::cout << "created descriptors" << std::endl;
	createPipelineStateManagement();
	std::cout << "created pipeline state management" <<std::endl;
	createPushConstants();
}

void VulkanRenderer::createCommandPool()
{
	VulkanDevice* deviceObj = application->deviceObj;
	VkCommandPoolCreateInfo cmdPoolInfo;
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext = nullptr;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = deviceObj->graphicsQueueWithPresentIndex;

	vkCreateCommandPool(deviceObj->device, &cmdPoolInfo, nullptr, &cmdPool);
	//vk::UniqueDevice device;
	//device->createCommandPool(cmdPoolInfo);
}

void VulkanRenderer::buildSwapChainAndDepthImage()
{
	// Get the appropriate queue to submit the command into
	deviceObj->getDeviceQueue();

	// Create swapchain and get the color images
	swapChainObj->createSwapChain(cmdDepthImage);

	// Create the depth image
	createDepthImage();
}

void VulkanRenderer::prepare()
{
	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->prepare();
	}
}

bool VulkanRenderer::render()
{
	// Do some more linux stuff here
	//std::cout << "VulkanRenderer::render()\n";
	for (VulkanDrawable* drawableObj : drawableList)
	{
		if (!application->isResizing)
		{
			drawableObj->render();
		}

	}
	//glfwPollEvents();
	return false;//glfwWindowShouldClose(window);
}

void VulkanRenderer::createDepthImage()
{
	VkResult result;
	bool pass;

	VkImageCreateInfo imageInfo;

	// If format undefined, fall back to 16bit value
	if (Depth.format == VK_FORMAT_UNDEFINED)
	{
		Depth.format = VK_FORMAT_D16_UNORM;
	}
	const VkFormat depthFormat = Depth.format;

	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(*deviceObj->gpu, depthFormat, &props);

	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else
	{
		std::cout << "Unsupported depth format, try other depth formats.\n";
		exit(-1);
	}

	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthFormat;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = NUM_SAMPLES;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.flags = 0;

	result = vkCreateImage(deviceObj->device, &imageInfo, nullptr, &Depth.image);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memRqrmnt;
	vkGetImageMemoryRequirements(deviceObj->device, Depth.image, &memRqrmnt);

	VkMemoryAllocateInfo memAlloc;
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = nullptr;
	memAlloc.allocationSize = 0;
	memAlloc.memoryTypeIndex = 0;
	memAlloc.allocationSize = memRqrmnt.size;
	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, 0, &memAlloc.memoryTypeIndex);

	result = vkAllocateMemory(deviceObj->device, &memAlloc, nullptr, &Depth.mem);
	assert(result == VK_SUCCESS);

	result = vkBindImageMemory(deviceObj->device, Depth.image, Depth.mem, 0);
	assert(result == VK_SUCCESS);

	// Set image layout
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdDepthImage);
	CommandBufferMgr::beginCommandBuffer(cmdDepthImage);
	{
		VkImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		subresourceRange.baseArrayLayer = 0;
		setImageLayout(Depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, subresourceRange, cmdDepthImage);
	}
	CommandBufferMgr::endCommandBuffer(cmdDepthImage);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdDepthImage);

	VkImageViewCreateInfo imgViewInfo;
	imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewInfo.pNext = nullptr;
	imgViewInfo.flags = 0;
	imgViewInfo.image = VK_NULL_HANDLE;
	imgViewInfo.format = depthFormat;
	imgViewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
	imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imgViewInfo.subresourceRange.baseMipLevel = 0;
	imgViewInfo.subresourceRange.levelCount = 1;
	imgViewInfo.subresourceRange.baseArrayLayer = 0;
	imgViewInfo.subresourceRange.layerCount = 1;
	imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	if (depthFormat == VK_FORMAT_D16_UNORM_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT || depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		imgViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	imgViewInfo.image = Depth.image;
	result = vkCreateImageView(deviceObj->device, &imgViewInfo, nullptr, &Depth.view);
	assert(result == VK_SUCCESS);
}

void VulkanRenderer::setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange, const VkCommandBuffer& cmd)
{
	assert(cmd != VK_NULL_HANDLE);
	assert(deviceObj->queue != VK_NULL_HANDLE);

	VkImageMemoryBarrier imgMemoryBarrier;
	imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemoryBarrier.pNext = nullptr;
	imgMemoryBarrier.srcAccessMask = 0;
	imgMemoryBarrier.dstAccessMask = 0;
	imgMemoryBarrier.oldLayout = oldImageLayout;
	imgMemoryBarrier.newLayout = newImageLayout;
	imgMemoryBarrier.image = image;
	//imgMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	//imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
	//imgMemoryBarrier.subresourceRange.levelCount = 1;
	//imgMemoryBarrier.subresourceRange.layerCount = 1;
	imgMemoryBarrier.subresourceRange = subresourceRange;
	imgMemoryBarrier.srcQueueFamilyIndex = deviceObj->graphicsQueueFamilyIndex;
	imgMemoryBarrier.dstQueueFamilyIndex = deviceObj->graphicsQueueFamilyIndex;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imgMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}


	VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // TODO validation layers are MUCH more picky about the stage used here
	switch (newImageLayout)
	{
	// Ensure that anything that was copying from this image has completed
	// An image in this layout can only be used as the destination operand of the commands
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	// Ensure any copy or cpu writes to image are flushed
	// An image n this layout can only be used as a read-only shader resource
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dstStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	// An image in this layout can only be used as a frambuffer color attachment
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;
	// An image in this layout can only be used as a framebuffer depth/stencil attachment
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dstStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	}

	vkCmdPipelineBarrier(cmd, srcStages, dstStages, 0, 0, nullptr, 0, nullptr, 1, &imgMemoryBarrier);
}

void VulkanRenderer::createRenderPass(bool includeDepth, bool clear)
{
	// Dependency on VulkanSwapChain::creatSwapChain() to get color image and
	// VulkanRenderer::createDepthImage() to get the depth image
	VkResult result;
	// Attach the color buffer and depth buffer as attachments to the render pass instance
	VkAttachmentDescription attachments[2];
	attachments[0].format = swapChainObj->scPublicVars.format;
	attachments[0].samples = NUM_SAMPLES;
	attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].flags = 0;

	if (includeDepth)
	{
		attachments[1].flags = 0;
		attachments[1].format = Depth.format;
		attachments[1].samples = NUM_SAMPLES;
		attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Define color buffer attachment binding point and layout info
	VkAttachmentReference colorReference;
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // TODO is this wrong too?

	// Define depth buffer attachment binding point and layout info
	VkAttachmentReference depthReference;
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Specify the attachments - color, depth, resolve, preserver, etc.
	VkSubpassDescription subpass;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = includeDepth ? &depthReference : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	// Specify the attachment and subpass associate with render pass
	VkRenderPassCreateInfo rpInfo;
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpInfo.pNext = nullptr;
	rpInfo.flags = 0;
	rpInfo.attachmentCount = includeDepth ? 2 : 1;
	rpInfo.pAttachments = attachments;
	rpInfo.subpassCount = 1;
	rpInfo.pSubpasses = &subpass;
	rpInfo.dependencyCount = 0;
	rpInfo.pDependencies = nullptr;

	result = vkCreateRenderPass(deviceObj->device, &rpInfo, nullptr, &renderPass);
	assert(result == VK_SUCCESS);
}

void VulkanRenderer::destroyRenderPass()
{
	vkDestroyRenderPass(deviceObj->device, renderPass, nullptr);
}

void VulkanRenderer::createFramebuffers(bool includeDepth, bool clear)
{
	// Dependency on createDepthBuffer(), createRenderPass(), and recordSwapChain()
	VkResult result;

	VkImageView attachments[2];
	attachments[1] = Depth.view;

	VkFramebufferCreateInfo fbInfo;
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = includeDepth ? 2 : 1;
	fbInfo.pAttachments = attachments;
	fbInfo.width = width;
	fbInfo.height = height;
	fbInfo.layers = 1;

	uint32_t i;
	framebuffers.clear();
	framebuffers.resize(swapChainObj->scPublicVars.swapchainImageCount);

	for (i = 0; i < swapChainObj->scPublicVars.swapchainImageCount; i++)
	{
		attachments[0] = swapChainObj->scPublicVars.colorBuffer[i].view;
		result = vkCreateFramebuffer(deviceObj->device, &fbInfo, nullptr, &framebuffers.at(i));
		assert(result == VK_SUCCESS);
	}
}

void VulkanRenderer::destroyFramebuffers()
{
	for (uint32_t i = 0; i < swapChainObj->scPublicVars.swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(deviceObj->device, framebuffers.at(i), nullptr);
	}
	framebuffers.clear();
}

void VulkanRenderer::createPipelineStateManagement()
{
	for (VulkanDrawable* drawableObj : drawableList)
	{
		std::cout << "Creating pipeline layout: " << &drawableObj->pipelineLayout << std::endl;
		drawableObj->createPipelineLayout();
	}

	// Create the pipeline cache
	pipelineObj.createPipelineCache();

	const bool depthPresent = true;
	for (VulkanDrawable* drawableObj : drawableList)
	{
		VkPipeline* pipeline = (VkPipeline*)malloc(sizeof(VkPipeline));
		if (pipelineObj.createPipeline(drawableObj, pipeline, &shaderObj, depthPresent))
		{
			pipelineList.push_back(pipeline);
			drawableObj->setPipeline(pipeline);
		}
		else
		{
			free(pipeline);
			pipeline = nullptr;
		}
	}
}

void VulkanRenderer::destroyPipeline()
{
	for (VkPipeline* pipeline : pipelineList)
	{
		vkDestroyPipeline(deviceObj->device, *pipeline, nullptr);
		free(pipeline);
	}
	pipelineList.clear();
}

void VulkanRenderer::createVertexBuffer()
{
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdVertexBuffer);
	CommandBufferMgr::beginCommandBuffer(cmdVertexBuffer);

	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->createVertexBuffer(cubeData.data(), sizeof(cubeData), sizeof(cubeData[0]), false);
	}
	CommandBufferMgr::endCommandBuffer(cmdVertexBuffer);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdVertexBuffer);
}

void VulkanRenderer::createIndexBuffer()
{
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdIndexBuffer);
	CommandBufferMgr::beginCommandBuffer(cmdIndexBuffer);

	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->createIndexBuffer(squareIndices.data(), sizeof(squareIndices), sizeof(squareIndices[0]));
	}
	CommandBufferMgr::endCommandBuffer(cmdIndexBuffer);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdIndexBuffer);
}

void VulkanRenderer::createShaders()
{
	if (application->isResizing)
		return;
	void* vertShaderCode;
	void* fragShaderCode;
	size_t sizeVert, sizeFrag;

	vertShaderCode = readFile("/home/nic/Documents/Projects/GraphicsPlayground/DrawTex-vert.spv", &sizeVert);
	fragShaderCode = readFile("/home/nic/Documents/Projects/GraphicsPlayground/DrawTex-frag.spv", &sizeFrag);

	shaderObj.buildShaderModuleWithSPV((uint32_t*)vertShaderCode, sizeVert, (uint32_t*)fragShaderCode, sizeFrag);
}

void VulkanRenderer::destroyDepthBuffer()
{
	vkDestroyImageView(deviceObj->device, Depth.view, nullptr);
	vkDestroyImage(deviceObj->device, Depth.image, nullptr);
	vkFreeMemory(deviceObj->device, Depth.mem, nullptr);
}

void VulkanRenderer::destroyDrawableVertexBuffer()
{
	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->destroyVertexBuffer();
	}
}

void VulkanRenderer::destroyRenderpass()
{
	vkDestroyRenderPass(deviceObj->device, renderPass, nullptr);
}

void VulkanRenderer::destroyCommandPool()
{
	VulkanDevice* deviceObj = application->deviceObj;

	vkDestroyCommandPool(deviceObj->device, cmdPool, nullptr);
}

void VulkanRenderer::update()
{
	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->update();
	}
}

void VulkanRenderer::createDescriptors()
{
	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->createDescriptorLayout(true);
		std::cout << "created descriptor layout" << std::endl;
		drawableObj->createDescriptor(true);
		std::cout << "created descriptor" << std::endl;
	}
}

void VulkanRenderer::createPushConstants()
{
	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdPushConstants);
	CommandBufferMgr::beginCommandBuffer(cmdPushConstants);

	enum ColorFlag
	{
		RED = 1,
		GREEN = 2,
		BLUE = 3,
		MIXED_COLOR = 4
	};

	float mixerValue = 0.3f;
	unsigned constColorRGBFlag = BLUE;

	struct
	{
		int flag;
		float mixer;
	} pushConstantLayout;
	pushConstantLayout.flag = BLUE;
	pushConstantLayout.mixer = mixerValue;
	//unsigned pushConstants[2];
	//pushConstants[0] = constColorRGBFlag;
	//memcpy(&pushConstants[1], &mixerValue, sizeof(float));

	int maxPushConstantSize = getDevice()->gpuProps.limits.maxPushConstantsSize;
	if (sizeof(pushConstantLayout) > maxPushConstantSize)
	{
		std::cout << "Push constant size is greater than expected, max allow size is " << maxPushConstantSize << std::endl;
		assert(0);
	}

	for (VulkanDrawable* drawableObj : drawableList)
	{
		std::cout << "Updating push constants: " << &drawableObj->pipelineLayout << std::endl;
		vkCmdPushConstants(cmdPushConstants, drawableObj->pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8, &pushConstantLayout);
	}

	CommandBufferMgr::endCommandBuffer(cmdPushConstants);
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdPushConstants);
}

void VulkanRenderer::createTextureLinear(const char* filename, TextureData* texture, VkImageUsageFlags imageUsageFlags, VkFormat format)
{
	// Load image
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filename, &width, &height,&channels, STBI_rgb_alpha);
	texture->width = static_cast<uint32_t>(width);
	texture->height = static_cast<uint32_t>(height);
	texture->mipMapLevels = 1;

	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.width = texture->width;
	imageCreateInfo.extent.height = texture->height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = texture->mipMapLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;

	VkResult result;
	result = vkCreateImage(deviceObj->device, &imageCreateInfo, nullptr, &texture->image);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(deviceObj->device, texture->image, &memoryRequirements);

	VkMemoryAllocateInfo& memAlloc = texture->allocInfo;
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = nullptr;
	memAlloc.allocationSize = memoryRequirements.size;
	memAlloc.memoryTypeIndex = 0;
	bool pass = deviceObj->memoryTypeFromProperties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &texture->allocInfo.memoryTypeIndex);
	assert(pass);

	result = vkAllocateMemory(deviceObj->device, &texture->allocInfo, nullptr, &texture->memory);
	assert(result == VK_SUCCESS);

	result = vkBindImageMemory(deviceObj->device, texture->image, texture->memory, 0);
	assert(result == VK_SUCCESS);

	VkImageSubresource subresource;
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	VkSubresourceLayout layout;
	uint8_t* data;

	vkGetImageSubresourceLayout(deviceObj->device, texture->image, &subresource, &layout);

	result = vkMapMemory(deviceObj->device, texture->memory, 0, texture->allocInfo.allocationSize, 0, (void**)&data);
	assert(result == VK_SUCCESS);

	// Load texture data into mapped buffer
	uint8_t* tempData = (uint8_t*)pixels;
	for (int y = 0; y < texture->height; y++)
	{
		size_t imageByteWidth = texture->width * 4;
		memcpy(data, tempData, imageByteWidth);
		tempData += imageByteWidth;

		data += layout.rowPitch;
	}

	vkUnmapMemory(deviceObj->device, texture->memory);

	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdTexture);
	CommandBufferMgr::beginCommandBuffer(cmdTexture);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipMapLevels;
	subresourceRange.layerCount = 1;
	subresourceRange.baseArrayLayer = 0;

	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	setImageLayout(texture->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, texture->imageLayout, subresourceRange, cmdTexture);

	CommandBufferMgr::endCommandBuffer(cmdTexture);

	// Fence to ensure copies finish before continuing
	VkFence fence;
	VkFenceCreateInfo fenceCi;
	fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCi.pNext = nullptr;
	fenceCi.flags = 0;
	vkCreateFence(deviceObj->device, &fenceCi, nullptr, &fence);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdTexture;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = 0;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdTexture, &submitInfo, fence);

	// Wait for max of 10 seconds
	vkWaitForFences(deviceObj->device, 1, &fence, VK_TRUE, 10000000000);
	vkDestroyFence(deviceObj->device, fence, nullptr);

	VkSamplerCreateInfo samplerCi;
	samplerCi.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCi.pNext = nullptr;
	samplerCi.flags = 0;
	samplerCi.magFilter = VK_FILTER_LINEAR;
	samplerCi.minFilter = VK_FILTER_LINEAR;
	samplerCi.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCi.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.mipLodBias = 0.0f;

	if (deviceObj->deviceFeatures.samplerAnisotropy == VK_TRUE)
	{
		samplerCi.anisotropyEnable = VK_TRUE;
		samplerCi.maxAnisotropy = 8;
	} else
	{
		samplerCi.anisotropyEnable = VK_FALSE;
		samplerCi.maxAnisotropy = 1;
	}
	samplerCi.compareOp = VK_COMPARE_OP_NEVER;
	samplerCi.compareEnable = VK_FALSE;
	samplerCi.minLod = 0.0f;
	samplerCi.maxLod = 0.0f;
	samplerCi.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCi.unnormalizedCoordinates = VK_FALSE;

	result = vkCreateSampler(deviceObj->device, &samplerCi, nullptr, &texture->sampler);
	assert(result == VK_SUCCESS);

	texture->descriptorImageInfo.sampler = texture->sampler;

	VkImageViewCreateInfo viewCi;
	viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCi.pNext = nullptr;
	viewCi.flags = 0;
	viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCi.format = format;
	viewCi.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCi.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCi.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCi.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCi.subresourceRange = subresourceRange;
	viewCi.image = texture->image;
	result = vkCreateImageView(deviceObj->device, &viewCi, nullptr, &texture->view);
	assert(result == VK_SUCCESS);

	texture->descriptorImageInfo.imageView = texture->view;
	texture->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->setTextures(texture);
	}
}

void VulkanRenderer::createTextureOptimal(const char* filename, TextureData* texture, VkImageUsageFlags imageUsageFlags, VkFormat format)
{
	std::cout << "starting loading texture optimal" << std::endl;
	// Load image
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filename, &width, &height,&channels, STBI_rgb_alpha);
	texture->width = static_cast<uint32_t>(width);
	texture->height = static_cast<uint32_t>(height);
	texture->mipMapLevels = 1;
	std::cout << "texture info - width: " << width << " | height: " << height << " | channels: " << channels << std::endl;

	VkBufferCreateInfo bufferCi;
	bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCi.pNext = nullptr;
	bufferCi.flags = 0;
	bufferCi.size = width * height * 4;
	bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCi.queueFamilyIndexCount = 0;
	bufferCi.pQueueFamilyIndices = nullptr;

	VkResult result;
	VkBuffer buffer;
	result = vkCreateBuffer(deviceObj->device, &bufferCi, nullptr, &buffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memRequirement;
	VkDeviceMemory bufferMemory;
	vkGetBufferMemoryRequirements(deviceObj->device, buffer, &memRequirement);

	VkMemoryAllocateInfo bufferMemAllocInfo;
	bufferMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemAllocInfo.pNext = nullptr;
	bufferMemAllocInfo.allocationSize = memRequirement.size;
	bufferMemAllocInfo.memoryTypeIndex = 0;
	deviceObj->memoryTypeFromProperties(memRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferMemAllocInfo.memoryTypeIndex);

	result = vkAllocateMemory(deviceObj->device, &bufferMemAllocInfo, nullptr, &bufferMemory);
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(deviceObj->device, buffer, bufferMemory, 0);
	assert(result == VK_SUCCESS);

	uint8_t* data;
	result = vkMapMemory(deviceObj->device, bufferMemory, 0, memRequirement.size, 0, (void**)&data);
	assert(result == VK_SUCCESS);

	memcpy(data, pixels, width * height * 4);
	vkUnmapMemory(deviceObj->device, bufferMemory);

	VkImageCreateInfo imageCi;
	imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCi.pNext = nullptr;
	imageCi.flags = 0;
	imageCi.imageType = VK_IMAGE_TYPE_2D;
	imageCi.format = format;
	imageCi.mipLevels = texture->mipMapLevels;
	imageCi.arrayLayers = 1;
	imageCi.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCi.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCi.extent = {texture->width, texture->height, 1};
	imageCi.usage = imageUsageFlags;

	// Make sure the transfer dst bit is set
	if (!(imageCi.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCi.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	result = vkCreateImage(deviceObj->device, &imageCi, nullptr, &texture->image);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements imageMemRequirement;
	vkGetImageMemoryRequirements(deviceObj->device, texture->image, &imageMemRequirement);

	VkMemoryAllocateInfo imageMemAllocInfo;
	imageMemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemAllocInfo.pNext = nullptr;
	imageMemAllocInfo.allocationSize = imageMemRequirement.size;
	imageMemAllocInfo.memoryTypeIndex = 0;
	deviceObj->memoryTypeFromProperties(imageMemRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imageMemAllocInfo.memoryTypeIndex);

	result = vkAllocateMemory(deviceObj->device, &imageMemAllocInfo, nullptr, &texture->memory);
	assert(result == VK_SUCCESS);

	result = vkBindImageMemory(deviceObj->device, texture->image, texture->memory, 0);
	assert(result == VK_SUCCESS);

	CommandBufferMgr::allocCommandBuffer(&deviceObj->device, cmdPool, &cmdTexture);
	CommandBufferMgr::beginCommandBuffer(cmdTexture);

	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipMapLevels;
	subresourceRange.layerCount = 1;
	subresourceRange.baseArrayLayer = 0;

	//texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	setImageLayout(texture->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, cmdTexture);

	std::vector<VkBufferImageCopy> bufferImgCopyList;
	uint32_t bufferOffset = 0;
	//for (uint32_t i = 0; i < texture->mipMapLevels; i++)
	{
		uint32_t i = 0;
		VkBufferImageCopy bufImgCopyItem;
		bufImgCopyItem.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufImgCopyItem.imageSubresource.mipLevel = i;
		bufImgCopyItem.imageSubresource.layerCount = 1;
		bufImgCopyItem.imageSubresource.baseArrayLayer = 0;
		bufImgCopyItem.imageExtent = {texture->width, texture->height, 1};
		bufImgCopyItem.bufferOffset = bufferOffset;
		bufImgCopyItem.imageOffset = {0, 0, 0};
		bufImgCopyItem.bufferRowLength = 0;
		bufImgCopyItem.bufferImageHeight = 0;
		bufferImgCopyList.push_back(bufImgCopyItem);
		bufferOffset += uint32_t(width * height);
	}

	vkCmdCopyBufferToImage(cmdTexture, buffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)bufferImgCopyList.size(), bufferImgCopyList.data());

	CommandBufferMgr::endCommandBuffer(cmdTexture);

	// Fence to ensure copies finish before continuing
	VkFence fence;
	VkFenceCreateInfo fenceCi;
	fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCi.pNext = nullptr;
	fenceCi.flags = 0;
	vkCreateFence(deviceObj->device, &fenceCi, nullptr, &fence);

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdTexture;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = 0;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdTexture, &submitInfo, fence);
	// Wait for max of 10 seconds
	vkWaitForFences(deviceObj->device, 1, &fence, VK_TRUE, 10000000000);
	vkDestroyFence(deviceObj->device, fence, nullptr);
	std::cout << "Submitted first command buffer for image creation" << std::endl;
	CommandBufferMgr::beginCommandBuffer(cmdTexture);
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	setImageLayout(texture->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->imageLayout, subresourceRange, cmdTexture);

	CommandBufferMgr::endCommandBuffer(cmdTexture);

	// Fence to ensure copies finish before continuing
	//VkFence fence;
	//VkFenceCreateInfo fenceCi;
	fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCi.pNext = nullptr;
	fenceCi.flags = 0;
	vkCreateFence(deviceObj->device, &fenceCi, nullptr, &fence);

	//VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdTexture;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = 0;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &cmdTexture, &submitInfo, fence);

	// Wait for max of 10 seconds
	vkWaitForFences(deviceObj->device, 1, &fence, VK_TRUE, 10000000000);
	vkDestroyFence(deviceObj->device, fence, nullptr);

	VkSamplerCreateInfo samplerCi;
	samplerCi.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCi.pNext = nullptr;
	samplerCi.flags = 0;
	samplerCi.magFilter = VK_FILTER_LINEAR;
	samplerCi.minFilter = VK_FILTER_LINEAR;
	samplerCi.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCi.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCi.mipLodBias = 0.0f;

	if (deviceObj->deviceFeatures.samplerAnisotropy == VK_TRUE)
	{
		samplerCi.anisotropyEnable = VK_TRUE;
		samplerCi.maxAnisotropy = 8;
	} else
	{
		samplerCi.anisotropyEnable = VK_FALSE;
		samplerCi.maxAnisotropy = 1;
	}
	samplerCi.compareOp = VK_COMPARE_OP_NEVER;
	samplerCi.compareEnable = VK_FALSE;
	samplerCi.minLod = 0.0f;
	samplerCi.maxLod = 0.0f;
	samplerCi.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCi.unnormalizedCoordinates = VK_FALSE;

	result = vkCreateSampler(deviceObj->device, &samplerCi, nullptr, &texture->sampler);
	assert(result == VK_SUCCESS);

	texture->descriptorImageInfo.sampler = texture->sampler;

	VkImageViewCreateInfo viewCi;
	viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCi.pNext = nullptr;
	viewCi.flags = 0;
	viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCi.format = format;
	viewCi.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCi.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCi.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCi.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCi.subresourceRange = subresourceRange;
	viewCi.image = texture->image;
	result = vkCreateImageView(deviceObj->device, &viewCi, nullptr, &texture->view);
	assert(result == VK_SUCCESS);

	texture->descriptorImageInfo.imageView = texture->view;
	//texture->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	for (VulkanDrawable* drawableObj : drawableList)
	{
		drawableObj->setTextures(texture);
	}

	std::cout << "finished loading texture optimal" << std::endl;
}
















