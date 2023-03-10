/*
 * Drawable.cpp
 *
 *  Created on: Mar 8, 2023
 *      Author: nic
 */

#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>

Drawable::Drawable(VulkanRenderer2& renderer, VkCommandPool& commandPool) : m_renderer(renderer)
{
	VkCommandBufferAllocateInfo commandBufferAI;
	commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAI.pNext = nullptr;
	commandBufferAI.commandPool = commandPool;
	commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAI.commandBufferCount = 1;

	VkResult result = vkAllocateCommandBuffers(m_renderer.device, &commandBufferAI, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer from combined queue family command pool\n");
	}

	VkSemaphoreCreateInfo semaphoreCI;
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;

	result = vkCreateSemaphore(m_renderer.device, &semaphoreCI, nullptr, &imageAvailableSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}
	result = vkCreateSemaphore(m_renderer.device, &semaphoreCI, nullptr, &renderFinishedSemaphore);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vkSemaphore\n");
	}
}

Drawable::~Drawable()
{
	vkDestroySemaphore(m_renderer.device, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(m_renderer.device, renderFinishedSemaphore, nullptr);
}

void Drawable::ExecuteCommandBuffer()
{
	vkWaitForFences(m_renderer.device, 1, &m_renderer.inFlightFence, VK_TRUE, UINT64_MAX); // TODO use a Drawable owned fence instead
	vkResetFences(m_renderer.device, 1, &m_renderer.inFlightFence);

	// Submit command buffer
	std::array<VkSemaphore, 1> waitSemaphores = {m_renderer.imageAvailableSemaphore}; // TODO use Drawable's instead
	std::array<VkSemaphore, 1> signalSemaphores = {m_renderer.renderFinishedSemaphore};
	std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphores.size();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = signalSemaphores.size();
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	VkResult result = vkQueueSubmit(m_renderer.combinedQueue, 1, &submitInfo, m_renderer.inFlightFence);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
	}
}

void Drawable::ClearWindow(VkImage& image)
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
	generalToClearBarrier.image = image;
	generalToClearBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
	//VK_PIPELINE_STAGE_TRANSFER_BIT
	// Use this for now to clear the image to a specific color
	VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};

	vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);

	VkImageMemoryBarrier clearToPresentBarrier;
	clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	clearToPresentBarrier.pNext = nullptr;
	clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	clearToPresentBarrier.image = image;
	clearToPresentBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

	vkEndCommandBuffer(commandBuffer);
}
void Drawable::RenderTriangle()
{

}


