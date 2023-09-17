/*
 * Buffer.cpp
 *
 *  Created on: Sep 15, 2023
 *      Author: nic
 */

#include "Buffer.hpp"
#include "VulkanRenderer.hpp"
#include <cstring>
#include <exception>

Buffer::Buffer(VulkanRenderer& rendererIn, const std::vector<Vertex>& vertexData) : renderer(rendererIn), buffer(VK_NULL_HANDLE), bufferMemory(VK_NULL_HANDLE)
{
	const size_t size = vertexData.size() * sizeof(Vertex);
	VkBufferCreateInfo bufferCi;
	bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCi.pNext = nullptr;
	bufferCi.flags = 0;
	bufferCi.size = size;
	bufferCi.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCi.queueFamilyIndexCount = 0;
	bufferCi.pQueueFamilyIndices = nullptr;

	VkResult result = vkCreateBuffer(renderer.device, &bufferCi, nullptr, &buffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer\n");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(renderer.device, buffer, &memRequirements);

	VkMemoryAllocateInfo memoryAi;
	memoryAi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAi.pNext = nullptr;
	memoryAi.allocationSize = memRequirements.size;
	memoryAi.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkAllocateMemory(renderer.device, &memoryAi, nullptr, &bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for buffer\n");
	}

	vkBindBufferMemory(renderer.device, buffer, bufferMemory, 0);

	const std::vector<Vertex> vertices = {
			{{0.0F, -0.5F}, {1.0F, 0.0F, 0.0F}},
			{{0.5F, 0.5F}, {1.0F, 1.0F, 1.0F}},
			{{-0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}}
	};

	// TODO (nic) this is a place that Vulkan and C++ butt heads
	void* data = nullptr;
	vkMapMemory(renderer.device, bufferMemory, 0, size, 0, &data);
	//std::copy(vertices.begin(), vertices.end(), data);
	std::memcpy(data, vertexData.data(), size);
	vkUnmapMemory(renderer.device, bufferMemory);
}

Buffer::~Buffer()
{
	vkDestroyBuffer(renderer.device, buffer, nullptr);
	vkFreeMemory(renderer.device, bufferMemory, nullptr);
}

uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	// Unfortunately, Vulkan using a bitmask prevents use of a standard algorithm cleanly
	for (uint32_t i = 0; i < renderer.gpu.memoryProperties.memoryTypeCount; i++)
	{
		if (static_cast<bool>(typeFilter & (1U << i)) &&
				(renderer.gpu.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type for buffer\n");
}





