/*
 * Camera.cpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#include "Camera.hpp"
#include "VulkanRenderer.hpp"
#include <cstring>
//#include <utility>

Camera::Camera(VulkanRenderer& rendererIn) :
			renderer(rendererIn),
			extent(0, 0),
			image(renderer),
			imageDataCpu(0, 0),
			commandBuffer(VK_NULL_HANDLE),
			transform(glm::mat4(1.0F)),
			perspective(glm::mat4(1.0F))
{
	//image(renderer);
	//imageDataCpu(0, 0);
}

Camera::Camera(VulkanRenderer& rendererIn, uint16_t width, uint16_t height) :
    		renderer(rendererIn),
			extent(width, height),
			image(renderer, extent, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
			imageDataCpu(width, height),
			commandBuffer(VK_NULL_HANDLE),
			transform(glm::mat4(1.0F)),
			perspective(glm::mat4(1.0F))
    {
        VkCommandBufferAllocateInfo commandBufferAI;
        commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAI.pNext = nullptr;
        commandBufferAI.commandPool = renderer.commandPool;
        commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAI.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers(renderer.device, &commandBufferAI, &commandBuffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffer from combined queue family command pool\n");
        }
    };

//Camera::Camera(Camera&& other) : renderer(other.renderer), image(std::move(other.image))
//{
//
//}

void Camera::clear(VkClearColorValue clearColor)
{
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    // Move the image into the correct layout
    // No? just use clearcolorimage
    VkImageMemoryBarrier generalToClearBarrier;
    generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    generalToClearBarrier.pNext = nullptr;
    generalToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.image = image.images[0];
    generalToClearBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
    // VK_PIPELINE_STAGE_TRANSFER_BIT
    //  Use this for now to clear the image to a specific color
    //const VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};

    vkCmdClearColorImage(commandBuffer, image.images[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);

//    VkImageMemoryBarrier clearToPresentBarrier;
//    clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    clearToPresentBarrier.pNext = nullptr;
//    clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//    clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//    clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//    clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    clearToPresentBarrier.image = image.images[0];
//    clearToPresentBarrier.subresourceRange = subresourceRange;
//
//    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

    vkEndCommandBuffer(commandBuffer);

    std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    const VkResult result = vkQueueSubmit(renderer.combinedQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit camera clearing buffer: \n" + std::to_string(result));
    }
    // TODO (nic) This will have issues with other things running in parallel
    vkQueueWaitIdle(renderer.combinedQueue);
}

ImageData& Camera::cpuData()
{
	Buffer buffer(renderer, sizeof(R8G8B8A8Texel) * extent.width * extent.height, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    // Move the image into the correct layout
    // No? just use clearcolorimage
    VkImageMemoryBarrier generalToClearBarrier;
    generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    generalToClearBarrier.pNext = nullptr;
    generalToClearBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.image = image.images[0];
    generalToClearBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
    // VK_PIPELINE_STAGE_TRANSFER_BIT
    //  Use this for now to clear the image to a specific color
    //const VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};
    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {extent.width, extent.height, 1};

    vkCmdCopyImageToBuffer(commandBuffer, image.images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.buffer, 1, &region);

    VkImageMemoryBarrier clearToPresentBarrier;
    clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    clearToPresentBarrier.pNext = nullptr;
    clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.image = image.images[0];
    clearToPresentBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

    vkEndCommandBuffer(commandBuffer);

    std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    const VkResult result = vkQueueSubmit(renderer.combinedQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit camera copy buffer: \n" + std::to_string(result));
    }
    // TODO (nic) This will have issues with other things running in parallel
    vkQueueWaitIdle(renderer.combinedQueue);

	// TODO (nic) this is a place that Vulkan and C++ butt heads
	void* data = nullptr;
	vkMapMemory(renderer.device, buffer.bufferMemory, 0, buffer.size, 0, &data);
	//std::copy(vertices.begin(), vertices.end(), data);
	std::memcpy(imageDataCpu.data.data(), data, buffer.size);
	vkUnmapMemory(renderer.device, buffer.bufferMemory);

	return imageDataCpu;
}








