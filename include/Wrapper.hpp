/*
 * Wrapper.hpp
 *
 *  Created on: Apr 5, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_WRAPPER_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_WRAPPER_HPP_

#include "Header.hpp"

class CommandBufferMgr
{
public:
	// Allocate memory for command buffers from the command pool
	static void allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo = nullptr);

	// Start command buffer recording
	static void beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo = nullptr);

	// End the command buffer recording
	static void endCommandBuffer(VkCommandBuffer cmdBuf);

	// Submit the command buffer for execution
	static void submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* cmdBufList, const VkSubmitInfo* submitInfo = nullptr, const VkFence& fence = VK_NULL_HANDLE);
};

void* readFile(const char* spvFileName, size_t* fileSize);

struct TextureData
{
	VkSampler sampler;
	VkImage image;
	VkImageLayout imageLayout;
	VkMemoryAllocateInfo allocInfo;
	VkDeviceMemory memory;
	VkImageView view;
	uint32_t mipMapLevels;
	uint32_t layterCount;
	uint32_t width;
	uint32_t height;
	VkDescriptorImageInfo descriptorImageInfo;
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_WRAPPER_HPP_ */
