/*
 * Buffer.hpp
 *
 *  Created on: Sep 15, 2023
 *      Author: nic
 */

#ifndef RENDERER_BUFFER_HPP_
#define RENDERER_BUFFER_HPP_

#include <array>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanRenderer;

struct Vertex
{
	std::array<float, 2> pos;
	std::array<float, 3> color;
};

class Buffer
{
public:
	Buffer(VulkanRenderer& renderer, const std::vector<Vertex>& vertexData);
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept = default;
	Buffer& operator=(Buffer&&) = delete;
	~Buffer();

	VulkanRenderer& renderer;
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

private:
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif /* RENDERER_BUFFER_HPP_ */
