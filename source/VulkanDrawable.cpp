/*
 * VulkanDrawable.cpp
 *
 *  Created on: Apr 16, 2022
 *      Author: nic
 */

#include "VulkanDrawable.hpp"
#include "VulkanApplication.hpp"
#include <thread>
#include <chrono>
#include <iostream>

VulkanDrawable::VulkanDrawable(VulkanRenderer* parent)
{
	// "Note: It's very important to initialize the member with 0 or respective value otherwise it will break the system"
	memset(&VertexBuffer, 0, sizeof(VertexBuffer));
	rendererObj = parent;

	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = nullptr;
	presentCompleteSemaphoreCreateInfo.flags = 0;
	vkCreateSemaphore(deviceObj->device, &presentCompleteSemaphoreCreateInfo, nullptr, &presentCompleteSemaphore);

	VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo;
	drawingCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	drawingCompleteSemaphoreCreateInfo.pNext = nullptr;
	drawingCompleteSemaphoreCreateInfo.flags = 0;
	vkCreateSemaphore(deviceObj->device, &drawingCompleteSemaphoreCreateInfo, nullptr, &drawingCompleteSemaphore);
}

void VulkanDrawable::createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture)
{
	VulkanApplication* appObj = VulkanApplication::GetInstance();
	VulkanDevice* deviceObj = appObj->deviceObj;

	VkResult result;
	bool pass;

	//Create the buffer resource metadata information
	VkBufferCreateInfo bufInfo;
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = nullptr;
	bufInfo.flags = 0;
	bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0; // TODO wtf
	bufInfo.pQueueFamilyIndices = nullptr;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &VertexBuffer.buffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, VertexBuffer.buffer, &memRqrmnt);

	// Allocate buffer memory
	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass == true);
	result = vkAllocateMemory(deviceObj->device, &allocInfo, nullptr, &VertexBuffer.memory);
	assert(result == VK_SUCCESS);

	VertexBuffer.bufferInfo.range = memRqrmnt.size;
	VertexBuffer.bufferInfo.offset = 0;

	// Map memory
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, VertexBuffer.memory, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// Copy data to mapped memory
	memcpy(pData, vertexData, dataSize);

	// Unmap device memory
	vkUnmapMemory(deviceObj->device, VertexBuffer.memory);

	// Bind buffer to device memory
	result = vkBindBufferMemory(deviceObj->device, VertexBuffer.buffer, VertexBuffer.memory, 0);
	assert(result == VK_SUCCESS);

	viIpBind.binding = 0;
	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	viIpBind.stride = dataStride;

	viIpAttrib[0].binding = 0;
	viIpAttrib[0].location = 0;
	viIpAttrib[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrib[0].offset = 0;
	viIpAttrib[1].binding = 0;
	viIpAttrib[1].location = 1;
	viIpAttrib[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
	viIpAttrib[1].offset = 16;
}

void VulkanDrawable::destroyVertexBuffer()
{
	VulkanDevice* deviceObj = rendererObj->getDevice();

	vkDestroyBuffer(deviceObj->device, VertexBuffer.buffer, nullptr);
	vkFreeMemory(deviceObj->device, VertexBuffer.memory, nullptr);
}

void VulkanDrawable::createIndexBuffer(const void* indexData, uint32_t dataSize, uint32_t dataStride)
{
	VulkanApplication* appObj = VulkanApplication::GetInstance();
	VulkanDevice* deviceObj = appObj->deviceObj;

	VkResult result;
	bool pass;

	//Create the buffer resource metadata information
	VkBufferCreateInfo bufInfo;
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = nullptr;
	bufInfo.flags = 0;
	bufInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufInfo.size = dataSize;
	bufInfo.queueFamilyIndexCount = 0; // TODO wtf
	bufInfo.pQueueFamilyIndices = nullptr;
	bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &IndexBuffer.buffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(deviceObj->device, IndexBuffer.buffer, &memRqrmnt);

	// Allocate buffer memory
	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRqrmnt.size;

	pass = deviceObj->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
	assert(pass == true);
	result = vkAllocateMemory(deviceObj->device, &allocInfo, nullptr, &IndexBuffer.memory);
	assert(result == VK_SUCCESS);

	IndexBuffer.bufferInfo.range = memRqrmnt.size;
	IndexBuffer.bufferInfo.offset = 0;

	// Map memory
	uint8_t* pData;
	result = vkMapMemory(deviceObj->device, IndexBuffer.memory, 0, memRqrmnt.size, 0, (void**)&pData);
	assert(result == VK_SUCCESS);

	// Copy data to mapped memory
	memcpy(pData, indexData, dataSize);

	// Unmap device memory
	vkUnmapMemory(deviceObj->device, IndexBuffer.memory);

	// Bind buffer to device memory
	result = vkBindBufferMemory(deviceObj->device, IndexBuffer.buffer, IndexBuffer.memory, 0);
	assert(result == VK_SUCCESS);

//	viIpBind.binding = 0;
//	viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//	viIpBind.stride = dataStride;
//
//	viIpAttrib[0].binding = 0;
//	viIpAttrib[0].location = 0;
//	viIpAttrib[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//	viIpAttrib[0].offset = 0;
//	viIpAttrib[1].binding = 0;
//	viIpAttrib[1].location = 1;
//	viIpAttrib[1].format = useTexture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
//	viIpAttrib[1].offset = 16;
}

void VulkanDrawable::destroyIndexBuffer()
{
	VulkanDevice* deviceObj = rendererObj->getDevice();

	vkDestroyBuffer(deviceObj->device, IndexBuffer.buffer, nullptr);
	vkFreeMemory(deviceObj->device, IndexBuffer.memory, nullptr);
}

void VulkanDrawable::prepare()
{
	VulkanDevice* deviceObj = rendererObj->getDevice();
	vecCmdDraw.resize(rendererObj->getSwapChain()->scPublicVars.colorBuffer.size());
	std::cout << "vecCmdDraw.size(): " << vecCmdDraw.size() << std::endl;
	// Allocate command buffer per swapchain color image
	for (int i = 0; i < rendererObj->getSwapChain()->scPublicVars.colorBuffer.size(); i++)
	{
		// Allocate, create, and start command buffer recording
		CommandBufferMgr::allocCommandBuffer(&deviceObj->device, *rendererObj->getCommandPool(), &vecCmdDraw[i]);
		CommandBufferMgr::beginCommandBuffer(vecCmdDraw[i]);

		// Create the render pass instance
		recordCommandBuffer(i, &vecCmdDraw[i]);

		// Finish command buffer recording
		CommandBufferMgr::endCommandBuffer(vecCmdDraw[i]);
	}
}

void VulkanDrawable::render()
{
	VulkanDevice* deviceObj = rendererObj->getDevice();
	VulkanSwapChain* swapChainObj = rendererObj->getSwapChain();
	uint32_t& currentColorImage = swapChainObj->scPublicVars.currentColorBuffer;
	VkSwapchainKHR& swapChain = swapChainObj->scPublicVars.swapChain;

	// vkAcquireNextImageKHR requires either a valid fence or semaphore
	//VkSemaphore presentCompleteSemaphore;


	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Get index of next available swapchain image
	VkResult result = swapChainObj->fpAcquireNextImageKHR(deviceObj->device, swapChain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentColorImage);

	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.commandBufferCount = (uint32_t) sizeof(&vecCmdDraw[currentColorImage]) / sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers = &vecCmdDraw[currentColorImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &drawingCompleteSemaphore;

	//std::cout << "currentColorImage index: " << currentColorImage << std::endl;
	CommandBufferMgr::submitCommandBuffer(deviceObj->queue, &vecCmdDraw[currentColorImage], &submitInfo);

	// Present the image in the window
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &drawingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &currentColorImage;
	presentInfo.pResults = nullptr;
	result = swapChainObj->fpQueuePresentKHR(deviceObj->queue, &presentInfo);
	if (result == VK_ERROR_DEVICE_LOST)
	{
		std::cout << "Error: lost device at present time" << std::endl;
	} else if (result == VK_ERROR_SURFACE_LOST_KHR)
	{
		std::cout << "Error: lost surface at present time" << std::endl;
	} else if (result == VK_SUBOPTIMAL_KHR)
	{
		std::cout << "Error: pass, but suboptimal khr" << std::endl;
	} else if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		std::cout << "Error: out of date KHR" << std::endl;
	} else if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
	{
		std::cout << "Error: lost full screen exclusive mode" << std::endl;
	}
	assert(result == VK_SUCCESS || result == VK_ERROR_OUT_OF_DATE_KHR);
}

void VulkanDrawable::recordCommandBuffer(int currentImage, VkCommandBuffer* cmdDraw)
{
	// Specify clear color value
	VkClearValue clearValues[2];
//	switch (currentImage)
//	{
//	case 0:
//		clearValues[0].color = {1.0f, 0.0f, 0.0f, 1.0f};
//		break;
//	case 1:
//		clearValues[0].color = {0.0f, 1.0f, 0.0f, 1.0f};
//		break;
//	case 2:
//		clearValues[0].color = {0.0f, 0.0f, 1.0f, 1.0f};
//		break;
//	default:
//		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
//		break;
//	}
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rpBegin;
	rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBegin.pNext = nullptr;
	rpBegin.renderPass = rendererObj->renderPass;
	rpBegin.framebuffer = rendererObj->framebuffers[currentImage];
	rpBegin.renderArea.extent.width = rendererObj->width;
	rpBegin.renderArea.extent.height = rendererObj->height;
	rpBegin.renderArea.offset.x = 0;
	rpBegin.renderArea.offset.y = 0;
	rpBegin.clearValueCount = 2;
	rpBegin.pClearValues = clearValues;

	vkCmdBeginRenderPass(*cmdDraw, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

	vkCmdBindDescriptorSets(*cmdDraw, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets.data(), 0, nullptr);

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*cmdDraw, 0, 1, &VertexBuffer.buffer, offsets);

	//vkCmdBindIndexBuffer(*cmdDraw, IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

	initViewports(cmdDraw);
	initScissors(cmdDraw);

	// TODO test for push constants here
	enum ColorFlag
	{
		RED = 1,
		GREEN = 2,
		BLUE = 3,
		MIXED_COLOR = 4
	};

	float mixerValue = 0.8f;
	unsigned constColorRGBFlag = BLUE;

	struct
	{
		int flag;
		float mixer;
	} pushConstantLayout;
	pushConstantLayout.flag = MIXED_COLOR;
	pushConstantLayout.mixer = mixerValue;
	//unsigned pushConstants[2];
	//pushConstants[0] = constColorRGBFlag;
	//memcpy(&pushConstants[1], &mixerValue, sizeof(float));

//	int maxPushConstantSize = getDevice()->gpuProps.limits.maxPushConstantsSize;
//	if (sizeof(pushConstantLayout) > maxPushConstantSize)
//	{
//		std::cout << "Push constant size is greater than expected, max allow size is " << maxPushConstantSize << std::endl;
//		assert(0);
//	}

	//std::cout << "Updating push constants: " << &pipelineLayout << std::endl;
	vkCmdPushConstants(*cmdDraw, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8, &pushConstantLayout); // This seems to need to be here instead of separate. same command list? same time as draw?

	vkCmdDraw(*cmdDraw, 36, 1, 0, 0);
	//vkCmdDrawIndexed(*cmdDraw, 6, 1, 0, 0, 0);

	vkCmdEndRenderPass(*cmdDraw);
}

void VulkanDrawable::initViewports(VkCommandBuffer* cmd)
{
	viewport.height = (float) rendererObj->height;
	viewport.width = (float) rendererObj->width;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(*cmd, 0, NUMBER_OF_VIEWPORTS, &viewport);
}

void VulkanDrawable::initScissors(VkCommandBuffer* cmd)
{
	scissor.extent.width = rendererObj->width;
	scissor.extent.height = rendererObj->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(*cmd, 0, NUMBER_OF_SCISSORS, &scissor);
}

void VulkanDrawable::createDescriptorLayout(bool useTexture)
{
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = nullptr;

	if (useTexture)
	{
		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = useTexture ? 2 : 1;
	layoutCreateInfo.pBindings = layoutBindings;

	std::cout << "layoutCreateInfo.bindingCout: " << layoutCreateInfo.bindingCount << std::endl;
	VkResult result;
	descriptorLayouts.resize(1);
	result = vkCreateDescriptorSetLayout(deviceObj->device, &layoutCreateInfo, nullptr, descriptorLayouts.data());
	assert(result == VK_SUCCESS);
}

void VulkanDrawable::createPipelineLayout()
{
	const unsigned pushConstantRangeCount = 1;
	VkPushConstantRange pushConstantRanges[pushConstantRangeCount];
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[0].offset = 0;
	pushConstantRanges[0].size = 8;

	VkPipelineLayoutCreateInfo plCreateInfo;
	plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plCreateInfo.pNext = nullptr;
	plCreateInfo.flags = 0;
	plCreateInfo.pushConstantRangeCount = 1;
	plCreateInfo.pPushConstantRanges = pushConstantRanges;
	plCreateInfo.setLayoutCount = (uint32_t)descriptorLayouts.size();
	plCreateInfo.pSetLayouts = descriptorLayouts.data();

	VkResult result;
	result = vkCreatePipelineLayout(deviceObj->device, &plCreateInfo, nullptr, &pipelineLayout);
	assert(result == VK_SUCCESS);
}

void VulkanDrawable::createDescriptorPool(bool useTexture)
{
	VkResult result;

	VkDescriptorPoolSize descriptorTypePool[2];
	descriptorTypePool[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypePool[0].descriptorCount = 1;

	if (useTexture)
	{
		descriptorTypePool[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorTypePool[1].descriptorCount = 1;
	}

	VkDescriptorPoolCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = 1;
	createInfo.poolSizeCount = useTexture ? 2 : 1;
	createInfo.pPoolSizes = descriptorTypePool;

	result = vkCreateDescriptorPool(deviceObj->device, &createInfo, nullptr, &descriptorPool);
	assert(result == VK_SUCCESS);
}

void VulkanDrawable::createDescriptorResources()
{
	createUniformBuffer();
}

void VulkanDrawable::createUniformBuffer()
{
	VkResult result;
	bool pass;
	Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	View = glm::lookAt(glm::vec3(10, 3, 10), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
	Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;

	VkBufferCreateInfo buffInfo;
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.pNext = nullptr;
	buffInfo.flags = 0;
	buffInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffInfo.size = sizeof(MVP);
	buffInfo.queueFamilyIndexCount = 0;
	buffInfo.pQueueFamilyIndices = nullptr;
	buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(deviceObj->device, &buffInfo, nullptr, &UniformBuffer.buffer);
	assert(VK_SUCCESS == result);

	VkMemoryRequirements memRequirement;
	vkGetBufferMemoryRequirements(deviceObj->device, UniformBuffer.buffer, &memRequirement);

	VkMemoryAllocateInfo memAllocInfo;
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = nullptr;
	memAllocInfo.allocationSize = memRequirement.size;
	memAllocInfo.memoryTypeIndex = 0;
	pass = deviceObj->memoryTypeFromProperties(memRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);
	assert(pass);

	result = vkAllocateMemory(deviceObj->device, &memAllocInfo, nullptr, &UniformBuffer.memory);
	assert(VK_SUCCESS == result);

	result = vkMapMemory(deviceObj->device, UniformBuffer.memory, 0, memRequirement.size, 0, (void**)&UniformBuffer.pData);
	assert(VK_SUCCESS == result);

	memcpy(UniformBuffer.pData, &MVP, sizeof(MVP));

	UniformBuffer.mappedRange.resize(1);
	UniformBuffer.mappedRange[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	UniformBuffer.mappedRange[0].pNext = nullptr;
	UniformBuffer.mappedRange[0].memory = UniformBuffer.memory;
	UniformBuffer.mappedRange[0].offset = 0;
	UniformBuffer.mappedRange[0].size = sizeof(MVP);

	// Invalidate the range of mapped buffer in order to make it visible to the host
	vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &UniformBuffer.mappedRange[0]);

	result = vkBindBufferMemory(deviceObj->device, UniformBuffer.buffer, UniformBuffer.memory, 0);
	assert(VK_SUCCESS == result);

	// Update local data structure for housekeeping
	UniformBuffer.bufferInfo.buffer = UniformBuffer.buffer;
	UniformBuffer.bufferInfo.offset = 0;
	UniformBuffer.bufferInfo.range = sizeof(MVP);
	UniformBuffer.memRequirement = memRequirement;
}

void VulkanDrawable::createDescriptorSet(bool useTexture)
{
	VulkanPipeline* pipelineObj = rendererObj->getPipelineObject();
	VkResult result;

	VkDescriptorSetAllocateInfo dsAllocInfo[1];
	dsAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo[0].pNext = nullptr;
	dsAllocInfo[0].descriptorPool = descriptorPool;
	dsAllocInfo[0].descriptorSetCount = 1;
	dsAllocInfo[0].pSetLayouts = descriptorLayouts.data();

	descriptorSets.resize(1);
	result = vkAllocateDescriptorSets(deviceObj->device, dsAllocInfo, descriptorSets.data());
	assert(result == VK_SUCCESS);

	VkWriteDescriptorSet writes[2];
	//memset(&writes, 0, sizeof(writes));

	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].pNext = nullptr;
	writes[0].dstSet = descriptorSets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pImageInfo = nullptr;
	writes[0].pTexelBufferView = nullptr;
	writes[0].pBufferInfo = &UniformBuffer.bufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	if (useTexture)
	{
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].pNext = nullptr;
		writes[1].dstSet = descriptorSets[0];
		writes[1].dstBinding = 1;
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[1].pImageInfo = &textures->descriptorImageInfo;
		writes[1].pTexelBufferView = nullptr;
		writes[1].pBufferInfo = nullptr;
		writes[1].dstArrayElement = 0;
	}
	std::cout << "about to update descriptor sets" << std::endl;
	vkUpdateDescriptorSets(deviceObj->device, useTexture ? 2 : 1, writes, 0, nullptr);
	std::cout << "just called vkUpdateDescriptorSets" << std::endl;
}

void VulkanDrawable::update()
{
	VulkanDevice* deviceObj = rendererObj->getDevice();
	//uint8_t* pData;
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 Model = glm::mat4(1.0f);
	static float rot = 0;
	rot += 0.001f;
	Model = glm::rotate(Model, rot, glm::vec3(0.0, 1.0, 0.0)) * glm::rotate(Model, rot, glm::vec3(1.0, 1.0, 1.0));
	glm::mat4 MVP = Projection * View * Model;

	//VkResult result = vkMapMemory(deviceObj->device, UniformBuffer.memory, 0, UniformBuffer.memRequirement.size, 0, (void**)&pData);
	//assert(VK_SUCCESS == result);
	// TODO I am at least making it here before crashing
	VkResult result = vkInvalidateMappedMemoryRanges(deviceObj->device, 1, &UniformBuffer.mappedRange[0]);
	assert(VK_SUCCESS == result);
	memcpy(UniformBuffer.pData, &MVP, sizeof(MVP));
	result = vkFlushMappedMemoryRanges(deviceObj->device, 1, &UniformBuffer.mappedRange[0]);
	assert(VK_SUCCESS == result);
}

void VulkanDrawable::destroyUniformBuffer()
{
	vkUnmapMemory(deviceObj->device, UniformBuffer.memory);
	vkDestroyBuffer(deviceObj->device, UniformBuffer.buffer, nullptr);
	vkFreeMemory(deviceObj->device, UniformBuffer.memory, nullptr);
}

void VulkanDrawable::setTextures(TextureData* tex)
{
	textures = tex;
}
















