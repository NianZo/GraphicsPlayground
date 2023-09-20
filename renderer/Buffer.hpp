/*
 * Buffer.hpp
 *
 *  Created on: Sep 15, 2023
 *      Author: nic
 */

#ifndef RENDERER_BUFFER_HPP_
#define RENDERER_BUFFER_HPP_

#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanRenderer;

class Buffer
{
public:
	Buffer(VulkanRenderer& rendererIn, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept = default;
	Buffer& operator=(Buffer&&) = delete;
	~Buffer();

	void copyTo(Buffer& other) const;

	VulkanRenderer& renderer;
	size_t size;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

private:
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif /* RENDERER_BUFFER_HPP_ */
