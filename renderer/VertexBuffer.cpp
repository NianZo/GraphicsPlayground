/*
 * VertexBuffer.cpp
 *
 *  Created on: Sep 16, 2023
 *      Author: nic
 */

#include "VertexBuffer.hpp"
#include "VulkanRenderer.hpp"
#include <cstring>
#include <iostream>

VertexBuffer::VertexBuffer(VulkanRenderer& rendererIn, const std::vector<Vertex>& vertexData) :
    size(vertexData.size() * sizeof(Vertex)),
    buffer(rendererIn, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    renderer(rendererIn)
{
    const Buffer tempBuffer(renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // TODO (nic) this is a place that Vulkan and C++ butt heads
    void* data = nullptr;
    vkMapMemory(renderer.device, tempBuffer.bufferMemory, 0, size, 0, &data);
    // std::copy(vertices.begin(), vertices.end(), data);
    std::memcpy(data, vertexData.data(), size);
    vkUnmapMemory(renderer.device, tempBuffer.bufferMemory);

    tempBuffer.copyTo(buffer);
}

VertexBuffer::~VertexBuffer()
{
}
