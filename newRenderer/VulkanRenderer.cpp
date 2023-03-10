/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanDisplay.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>
#include <fstream>


VulkanRenderer2::VulkanRenderer2(RendererBase& base, VkSurfaceKHR& surface, uint32_t index, uint32_t width, uint32_t height) : rendererBase(base), gpu(base.physicalDevices[index])
{
	combinedQueueFamily = FindCombinedQueueFamily(surface);
	std::array<float, 1> queuePriorities = {0.0F};
	VkDeviceQueueCreateInfo queueCI;
	queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCI.pNext = nullptr;
	queueCI.flags = 0;
	queueCI.queueFamilyIndex = combinedQueueFamily;
	queueCI.queueCount = 1;
	queueCI.pQueuePriorities = queuePriorities.data();

	VkDeviceCreateInfo deviceCI;
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = nullptr;
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &queueCI;
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
	deviceCI.ppEnabledExtensionNames = deviceExtensionNames.data();
	deviceCI.pEnabledFeatures = nullptr;

	VkResult result = vkCreateDevice(gpu.physicalDevice, &deviceCI, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan device\n");
	}

	vkGetDeviceQueue(device, combinedQueueFamily, 0, &combinedQueue);

	VkCommandPoolCreateInfo commandPoolCI;
	commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCI.pNext = nullptr;
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCI.queueFamilyIndex = combinedQueueFamily;

	result = vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool for combined queue family\n");
	}

	VkCommandBufferAllocateInfo commandBufferAI;
	commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAI.pNext = nullptr;
	commandBufferAI.commandPool = commandPool;
	commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAI.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer from combined queue family command pool\n");
	}

	VkSemaphoreCreateInfo semaphoreCI;
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;

	result = vkCreateSemaphore(device, &semaphoreCI, nullptr, &imageAvailableSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}
	result = vkCreateSemaphore(device, &semaphoreCI, nullptr, &renderFinishedSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}

	VkFenceCreateInfo fenceCI;
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	result = vkCreateFence(device, &fenceCI, nullptr, &inFlightFence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fence\n");
	}

	display = std::make_unique<VulkanDisplay>(this, surface, width, height);
}

VulkanRenderer2::~VulkanRenderer2()
{
	//delete display;
	vkDeviceWaitIdle(device);
	display.reset();

	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroyFence(device, inFlightFence, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}

uint32_t VulkanRenderer2::FindCombinedQueueFamily(VkSurfaceKHR& surface)
{
	for (uint32_t i = 0; i < gpu.queueFamilyProperties.size(); i++)
	{
		VkBool32 isSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physicalDevice, i, surface, &isSupported);
		if ((gpu.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && isSupported)
		{
			return i;
		}
	}
	throw std::runtime_error("No combined queue family found\n");
}

void VulkanRenderer2::Render()
{
	Drawable drawable(*this, commandPool);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, display.get()->swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkImage& image = display.get()->image.get()->images[imageIndex];

//	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
//	vkResetFences(device, 1, &inFlightFence);

	drawable.ClearWindow(image);

	drawable.ExecuteCommandBuffer();



//	// Eventually command buffer recording should be done in a RendererDescription or Drawable object
//	vkResetCommandBuffer(commandBuffer, 0);
//	VkCommandBufferBeginInfo beginInfo;
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.pNext = nullptr;
//	beginInfo.flags = 0;
//	beginInfo.pInheritanceInfo = nullptr;
//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//	VkImageSubresourceRange subresourceRange;
//	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	subresourceRange.baseMipLevel = 0;
//	subresourceRange.levelCount = 1;
//	subresourceRange.baseArrayLayer = 0;
//	subresourceRange.layerCount = 1;
//
//	// Move the image into the correct layout
//	// No? just use clearcolorimage
//	VkImageMemoryBarrier generalToClearBarrier;
//	generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//	generalToClearBarrier.pNext = nullptr;
//	generalToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//	generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//	generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	generalToClearBarrier.image = image;
//	generalToClearBarrier.subresourceRange = subresourceRange;
//
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
//	//VK_PIPELINE_STAGE_TRANSFER_BIT
//	// Use this for now to clear the image to a specific color
//	VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};
//
//	vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);
//
//	VkImageMemoryBarrier clearToPresentBarrier;
//	clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//	clearToPresentBarrier.pNext = nullptr;
//	clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//	clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//	clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//	clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//	clearToPresentBarrier.image = image;
//	clearToPresentBarrier.subresourceRange = subresourceRange;
//
//	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
//
//
//	vkEndCommandBuffer(commandBuffer);
//
//	// Submit command buffer
	//std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore};
	std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
//	std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
//
//	VkSubmitInfo submitInfo;
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.pNext = nullptr;
//	submitInfo.waitSemaphoreCount = waitSemaphores.size();
//	submitInfo.pWaitSemaphores = waitSemaphores.data();
//	submitInfo.pWaitDstStageMask = waitStages.data();
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//	submitInfo.signalSemaphoreCount = signalSemaphores.size();
//	submitInfo.pSignalSemaphores = signalSemaphores.data();
//
//	VkResult result = vkQueueSubmit(combinedQueue, 1, &submitInfo, inFlightFence);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
//	}

	std::array<VkSwapchainKHR, 1> swapchains = {display.get()->swapchain};
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = signalSemaphores.size();
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = swapchains.size();
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(combinedQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present image\n");
	}
}

void VulkanRenderer2::RenderTriangle()
{
	VkResult result;

	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(device, display.get()->swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	// Eventually command buffer recording should be done in a RendererDescription or Drawable object
	vkResetCommandBuffer(commandBuffer, 0);
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

//	VkImageSubresourceRange subresourceRange;
//	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	subresourceRange.baseMipLevel = 0;
//	subresourceRange.levelCount = 1;
//	subresourceRange.baseArrayLayer = 0;
//	subresourceRange.layerCount = 1;
	auto vertShaderCode = readFile("DrawTriangle-vert.spv");
	auto fragShaderCode = readFile("DrawTriangle-frag.spv");

	VkShaderModuleCreateInfo vertShaderCI;
	vertShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertShaderCI.pNext = nullptr;
	vertShaderCI.flags = 0;
	vertShaderCI.codeSize = vertShaderCode.size();
	vertShaderCI.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
	VkShaderModule vertShaderModule;
	result = vkCreateShaderModule(device, &vertShaderCI, nullptr, &vertShaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex shader module\n");
	}
	VkPipelineShaderStageCreateInfo vertShaderStageCI;
	vertShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCI.pNext = nullptr;
	vertShaderStageCI.flags = 0;
	vertShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCI.module = vertShaderModule;
	vertShaderStageCI.pName = "main";
	vertShaderStageCI.pSpecializationInfo = nullptr;

	VkShaderModuleCreateInfo fragShaderCI;
	fragShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragShaderCI.pNext = nullptr;
	fragShaderCI.flags = 0;
	fragShaderCI.codeSize = fragShaderCode.size();
	fragShaderCI.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
	VkShaderModule fragShaderModule;
	result = vkCreateShaderModule(device, &fragShaderCI, nullptr, &fragShaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create fragment shader module\n");
	}
	VkPipelineShaderStageCreateInfo fragShaderStageCI;
	fragShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCI.pNext = nullptr;
	fragShaderStageCI.flags = 0;
	fragShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCI.module = fragShaderModule;
	fragShaderStageCI.pName = "main";
	fragShaderStageCI.pSpecializationInfo = nullptr;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageCI, fragShaderStageCI};

	std::vector<VkDynamicState> dynamicStates =
	{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineVertexInputStateCreateInfo vertexInput;
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.pNext = nullptr;
	vertexInput.flags = 0;
	vertexInput.vertexBindingDescriptionCount = 0;
	vertexInput.pVertexBindingDescriptions = nullptr;
	vertexInput.vertexAttributeDescriptionCount = 0;
	vertexInput.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = nullptr;
	inputAssembly.flags = 0;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

//	VkViewport viewport;
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(display->m_width);
//	viewport.height = static_cast<float>(display->m_height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	VkRect2D scissor;
//	scissor.offset = {0, 0};
//	scissor.extent = display->swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = nullptr;
	rasterizer.flags = 0;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling;
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = nullptr;
	multisampling.flags = 0;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Depth and stencil to be added later

	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;
	colorBlending.flags = 0;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutCI;
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.pNext = nullptr;
	pipelineLayoutCI.flags = 0;
	pipelineLayoutCI.setLayoutCount = 0;
	pipelineLayoutCI.pSetLayouts = nullptr;
	pipelineLayoutCI.pushConstantRangeCount = 0;
	pipelineLayoutCI.pPushConstantRanges = nullptr;
	result = vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout\n");
	}

	VkAttachmentDescription colorAttachment;
	colorAttachment.flags = 0;
	colorAttachment.format = display->swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{}; // zero initialize - this is a big structure
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPass renderPass;
	VkRenderPassCreateInfo renderPassCI;
	renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCI.pNext = nullptr;
	renderPassCI.flags = 0;
	renderPassCI.dependencyCount = 0;
	renderPassCI.pDependencies = nullptr;
	renderPassCI.attachmentCount = 1;
	renderPassCI.pAttachments = &colorAttachment;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpass;
	result = vkCreateRenderPass(device, &renderPassCI, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass\n");
	}

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCI.pNext = nullptr;
	pipelineCI.flags = 0;
	pipelineCI.stageCount = shaderStages.size();
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = &vertexInput;
	pipelineCI.pInputAssemblyState = &inputAssembly;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pRasterizationState = &rasterizer;
	pipelineCI.pMultisampleState = &multisampling;
	pipelineCI.pDepthStencilState = nullptr;
	pipelineCI.pColorBlendState = &colorBlending;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.layout = pipelineLayout;
	pipelineCI.renderPass = renderPass;
	pipelineCI.subpass = 0;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;
	VkPipeline graphicsPipeline;
	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline\n");
	}

	std::vector<VkFramebuffer> framebuffers;
	framebuffers.resize(display->image->imageViews.size());
	for (size_t i = 0; i < display->image->imageViews.size(); i++)
	{
		std::array<VkImageView, 1> attachments = {display->image->imageViews[i]};
		VkFramebufferCreateInfo framebufferCI;
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.pNext = nullptr;
		framebufferCI.flags = 0;
		framebufferCI.renderPass = renderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = attachments.data();
		framebufferCI.width = display->swapchainExtent.width;
		framebufferCI.height = display->swapchainExtent.height;
		framebufferCI.layers = 1;
		result = vkCreateFramebuffer(device, &framebufferCI, nullptr, &framebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer");
		}
	}
// start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
	VkRenderPassBeginInfo renderPassBI;
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.pNext = nullptr;
	renderPassBI.renderPass = renderPass;
	renderPassBI.framebuffer = framebuffers[imageIndex];
	renderPassBI.renderArea.offset = {0, 0};
	renderPassBI.renderArea.extent = display->swapchainExtent;
	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassBI.clearValueCount = 1;
	renderPassBI.pClearValues = &clearColor;
	vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(display->swapchainExtent.width);
	viewport.height = static_cast<float>(display->swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = display->swapchainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer
	std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore};
	std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
	std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	result = vkQueueSubmit(combinedQueue, 1, &submitInfo, inFlightFence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
	}

	std::array<VkSwapchainKHR, 1> swapchains = {display.get()->swapchain};
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = signalSemaphores.size();
	presentInfo.pWaitSemaphores = signalSemaphores.data();
	presentInfo.swapchainCount = swapchains.size();
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(combinedQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present image\n");
	}
}

void VulkanRenderer2::Resize(VkSurfaceKHR surface, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
{
	vkDeviceWaitIdle(device);
	display.reset();

	display = std::make_unique<VulkanDisplay>(this, surface, width, height);

}

std::vector<char> VulkanRenderer2::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open shader file: " + filename + "\n");
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
	file.close();

	return buffer;
}














