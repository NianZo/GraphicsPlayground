/*
 * Wrapper.cpp
 *
 *  Created on: Apr 5, 2022
 *      Author: nic
 */

#include "Wrapper.hpp"

void CommandBufferMgr::allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
	VkResult result;
	// If command information is available use it as it is
	if (commandBufferInfo)
	{
		result = vkAllocateCommandBuffers(*device, commandBufferInfo, cmdBuf);
		assert(!result);
		return;
	}

	// Default implementation, create the command buffer allocation info
	// and use the supplied parameter into it
	VkCommandBufferAllocateInfo cmdInfo;
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.pNext = nullptr;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);
	result = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
	assert(!result);
}

void CommandBufferMgr::beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
	VkResult result;
	// If the user has specified the custom command buffer use it
	if (inCmdBufInfo)
	{
		result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
		assert(result == VK_SUCCESS);
		return;
	}

	// otherwise, use the default implementation
	VkCommandBufferInheritanceInfo cmdBufInheritInfo;
	cmdBufInheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmdBufInheritInfo.pNext = nullptr;
	cmdBufInheritInfo.renderPass = VK_NULL_HANDLE;
	cmdBufInheritInfo.subpass = 0;
	cmdBufInheritInfo.framebuffer = VK_NULL_HANDLE;
	cmdBufInheritInfo.occlusionQueryEnable = VK_FALSE;
	cmdBufInheritInfo.queryFlags = 0;
	cmdBufInheritInfo.pipelineStatistics = 0;
	VkCommandBufferBeginInfo cmdBufInfo;
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;
	cmdBufInfo.flags = 0;
	cmdBufInfo.pInheritanceInfo = &cmdBufInheritInfo;

	result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);
	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::endCommandBuffer(VkCommandBuffer cmdBuf)
{
	VkResult result;
	result = vkEndCommandBuffer(cmdBuf);
	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* cmdBufList, const VkSubmitInfo* submitInfo, const VkFence& fence)
{
	VkResult result;
	// If submit information is available use it as it is.
	// This assumes that the commands are already specified
	// in the structure, hence ignore command buffer
	if (submitInfo)
	{
		result = vkQueueSubmit(queue, 1, submitInfo, fence);
		assert(!result);
		result = vkQueueWaitIdle(queue);
		assert(!result);
		return;
	}

	// else create the submit info with specified buffer commands
	VkSubmitInfo submitInfoLocal;
	submitInfoLocal.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfoLocal.pNext = nullptr;
	submitInfoLocal.waitSemaphoreCount = 0;
	submitInfoLocal.pWaitSemaphores = nullptr;
	submitInfoLocal.pWaitDstStageMask = nullptr;
	submitInfoLocal.commandBufferCount = (uint32_t) sizeof(cmdBufList) / sizeof(VkCommandBuffer);
	submitInfoLocal.pCommandBuffers = cmdBufList;
	submitInfoLocal.signalSemaphoreCount = 0;
	submitInfoLocal.pSignalSemaphores = nullptr;

	result = vkQueueSubmit(queue, 1, &submitInfoLocal, fence);
	assert(!result);
	result = vkQueueWaitIdle(queue);
	assert(!result);
}

void* readFile(const char* spvFileName, size_t* fileSize)
{
	FILE* fp = fopen(spvFileName, "rb");
	if (!fp)
	{
		return nullptr;
	}

	size_t retval;
	long int size;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void* spvShader = malloc(size+1); // for \0 null terminator
	memset(spvShader, 0 , size+1);

	retval = fread(spvShader, size, 1, fp);
	assert(retval == 1);

	*fileSize = size;
	fclose(fp);
	return spvShader;
}




















