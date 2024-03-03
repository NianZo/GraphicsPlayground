/*
 * Buffer.cpp
 *
 *  Created on: Sep 15, 2023
 *      Author: nic
 */

#include "Buffer.hpp"
#include "VulkanRenderer.hpp"
#include <exception>
#include <iostream>

Buffer::Buffer(VulkanRenderer& rendererIn, size_t sizeIn, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) :
    renderer(rendererIn),
    size(sizeIn),
    buffer(VK_NULL_HANDLE),
    bufferMemory(VK_NULL_HANDLE)
{
    VkBufferCreateInfo bufferCi;
    bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCi.pNext = nullptr;
    bufferCi.flags = 0;
    bufferCi.size = size;
    bufferCi.usage = usage;
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
    memoryAi.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    result = vkAllocateMemory(renderer.device, &memoryAi, nullptr, &bufferMemory);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate memory for buffer\n");
    }

    vkBindBufferMemory(renderer.device, buffer, bufferMemory, 0);
}

Buffer::~Buffer()
{
    vkDestroyBuffer(renderer.device, buffer, nullptr);
    vkFreeMemory(renderer.device, bufferMemory, nullptr);
}

Buffer::Buffer(Buffer&& other) noexcept :
    renderer(other.renderer),
    size(other.size),
    buffer(other.buffer),
    bufferMemory(other.bufferMemory)
{
    other.buffer = VK_NULL_HANDLE;
    other.bufferMemory = VK_NULL_HANDLE;
}

void Buffer::copyTo(Buffer& other) const
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer.commandPool; // TODO (nic) should use a separate pool with the one-time-use/transient flag
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer copyBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(renderer.device, &allocInfo, &copyBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffer to copy buffer\n");
    }

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(copyBuffer, &beginInfo);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(copyBuffer, buffer, other.buffer, 1, &copyRegion);

    vkEndCommandBuffer(copyBuffer);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    result = vkQueueSubmit(renderer.combinedQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit copy command buffer for buffer creation\n");
    }
    result = vkQueueWaitIdle(renderer.combinedQueue);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to wait for queu to idle for buffer creation\n");
    }

    vkFreeCommandBuffers(renderer.device, renderer.commandPool, 1, &copyBuffer);
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
