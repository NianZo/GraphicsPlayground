/*
 * UniformBuffer.cpp
 *
 *  Created on: Sep 20, 2023
 *      Author: nic
 */

#include "UniformBuffer.hpp"
#include "VulkanRenderer.hpp"

UniformBuffer::UniformBuffer(VulkanRenderer& rendererIn, size_t sizeIn) :
	size(sizeIn),
	buffer(rendererIn, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	data(nullptr),
	renderer(rendererIn)
{
	vkMapMemory(renderer.device, buffer.bufferMemory, 0, size, 0, &data);
}
