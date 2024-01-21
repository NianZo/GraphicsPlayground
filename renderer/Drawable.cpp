/*
 * Drawable.cpp
 *
 *  Created on: Mar 8, 2023
 *      Author: nic
 */

#define GLM_FORCE_RADIANS // TODO (nic) this probably shouldn't go in this file
#include "Buffer.hpp"
#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

std::vector<char> readFile(const std::string& filename);
// clang-tidy doesn't understand that Vulkan initializes several of the class members
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
Drawable::Drawable(Scene& scene, GraphicsPipelineDescriptor& pipelineDescriptor) :
		transform(glm::mat4(1.0F)),
		m_renderer(scene.renderer),
		pipelineDescriptors(pipelineDescriptor),
		vertexBuffer(m_renderer, pipelineDescriptor.vertexData),
		indexBuffer(m_renderer, pipelineDescriptor.indexData),
		depthImage(m_renderer, scene.camera.extent, VulkanImage::findDepthFormat(m_renderer.gpu.physicalDevice), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		m_scene(scene)
{
	pipelineDescriptors.colorAttachment.format = m_scene.camera.image.m_format;
	pipelineDescriptors.depthAttachment.format = depthImage.m_format;

	pipelineDescriptors.viewports[0].width = static_cast<float>(scene.camera.extent.width);
	pipelineDescriptors.viewports[0].height = static_cast<float>(scene.camera.extent.height);

	pipelineDescriptors.scissors[0].extent = scene.camera.extent;

	VkCommandBufferAllocateInfo commandBufferAI;
	commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAI.pNext = nullptr;
	commandBufferAI.commandPool = m_renderer.commandPool;
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

	VkFenceCreateInfo fenceCI;
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	result = vkCreateFence(m_renderer.device, &fenceCI, nullptr, &inFlightFence);
	if (result != VK_SUCCESS)
	{
	    throw std::runtime_error("Failed to create fence\n");
	}

	uniformBuffers.emplace_back(m_renderer, sizeof(UniformBufferObject));

	pipelineStates.emplace_back(m_renderer.device, m_renderer, *this, pipelineDescriptors);

    //UniformBufferObject& ubo = renderer.scenes[0].drawables[0].ubo;
//    ubo.model = glm::mat4(1.0F);//glm::rotate(glm::mat4(1.0F), time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F));
//    ubo.view = glm::mat4(1.0F);//glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
//    ubo.proj = glm::mat4(1.0F);//glm::perspective(glm::radians(45.0F), static_cast<float>(renderer.display->swapchainExtent.width) / static_cast<float>(renderer.display->swapchainExtent.height), 0.1F, 10.0F);
//    ubo.proj[1][1] *= -1;
//    std::memcpy(uniformBuffers[0].data, &ubo, sizeof(UniformBufferObject));

	framebuffers.resize(m_scene.camera.image.imageViews.size());
	for (size_t i = 0; i < m_scene.camera.image.imageViews.size(); i++)
	{
	    std::array<VkImageView, 2> attachments = {m_scene.camera.image.imageViews[i], depthImage.imageViews[0]};
	    VkFramebufferCreateInfo framebufferCI;
	    framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebufferCI.pNext = nullptr;
	    framebufferCI.flags = 0;
	    framebufferCI.renderPass = pipelineStates[0].renderPass;
	    framebufferCI.attachmentCount = attachments.size();
	    framebufferCI.pAttachments = attachments.data();
	    framebufferCI.width = m_scene.camera.extent.width;
	    framebufferCI.height = m_scene.camera.extent.height;
	    framebufferCI.layers = 1;
	    result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[i]);
	    if (result != VK_SUCCESS)
	    {
	        throw std::runtime_error("Failed to create framebuffer");
	    }
	}
}

// clang-tidy doesn't understand that Vulkan initializes several of the class members
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
Drawable::Drawable(VulkanRenderer& renderer, VkCommandPool& commandPool, const GraphicsPipelineDescriptor& pipelineDescriptor) :
		m_renderer(renderer),
		pipelineDescriptors(pipelineDescriptor),
		vertexBuffer(renderer, pipelineDescriptor.vertexData),
		indexBuffer(renderer, pipelineDescriptor.indexData),
		depthImage(renderer, renderer.display->swapchainExtent, VulkanImage::findDepthFormat(renderer.gpu.physicalDevice), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
		m_scene(renderer.scenes[0])
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

    VkFenceCreateInfo fenceCI;
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(m_renderer.device, &fenceCI, nullptr, &inFlightFence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence\n");
    }

    for (uint32_t i = 0; i < m_renderer.display->surfaceCapabilities.minImageCount; i++)
    {
    	uniformBuffers.emplace_back(m_renderer, sizeof(UniformBufferObject));
    }

    pipelineStates.emplace_back(m_renderer.device, m_renderer, *this, pipelineDescriptor);

    framebuffers.resize(m_renderer.scenes[0].camera.image.imageViews.size());
    for (size_t i = 0; i < m_renderer.scenes[0].camera.image.imageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {m_renderer.scenes[0].camera.image.imageViews[i], depthImage.imageViews[0]};
        VkFramebufferCreateInfo framebufferCI;
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.pNext = nullptr;
        framebufferCI.flags = 0;
        framebufferCI.renderPass = pipelineStates[0].renderPass;
        framebufferCI.attachmentCount = attachments.size();
        framebufferCI.pAttachments = attachments.data();
        framebufferCI.width = m_renderer.display->swapchainExtent.width;
        framebufferCI.height = m_renderer.display->swapchainExtent.height;
        framebufferCI.layers = 1;
        result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

Drawable::~Drawable()
{
    vkDeviceWaitIdle(m_renderer.device);
    vkDestroySemaphore(m_renderer.device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_renderer.device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(m_renderer.device, inFlightFence, nullptr);
    for (VkFramebuffer& framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(m_renderer.device, framebuffer, nullptr);
    }
}

//Drawable::Drawable(Drawable&& other) noexcept
//{
//	if (this == &other)
//	{
//		return *this;
//	}
//
//	imageAvailableSemaphore = other.imageAvailableSemaphore;
//	renderFinishedSemaphore = other.renderFinishedSemaphore;
//	inFlightFence = other.inFlightFence;
//	m_renderer = other.m_renderer;
//	commandBuffer = other.commandBuffer;
//	pipelineDescriptors = other.pipelineDescriptors;
//	pipelineStates = std::move(other.pipelineStates);
//	framebuffers = std::move(other.framebuffers);
//
//	return *this;
//}

void Drawable::ExecuteCommandBuffer([[maybe_unused]]uint32_t imageIndex)
{
    // Should the queue execution happen in renderer instead of drawable?
    // Drawable should own fences and semaphores, but renderer owns the queues that this will be submitted to
//    vkWaitForFences(m_renderer.device, 1, &inFlightFence, VK_TRUE, UINT64_MAX); // TODO(nic) use a Drawable owned fence instead
//    vkResetFences(m_renderer.device, 1, &inFlightFence);

    // TODO (nic) I highly doubt this should be here...
//    static auto startTime = std::chrono::high_resolution_clock::now();
//    auto currentTime = std::chrono::high_resolution_clock::now();
//    const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
//    //UniformBufferObject ubo;
//    ubo.model = glm::rotate(glm::mat4(1.0F), time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F));
//    ubo.view = glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
//    ubo.proj = glm::perspective(glm::radians(45.0F), static_cast<float>(m_renderer.display->swapchainExtent.width) / static_cast<float>(m_renderer.display->swapchainExtent.height), 0.1F, 10.0F);
//    ubo.proj[1][1] *= -1;
//    std::memcpy(uniformBuffers[imageIndex].data, &ubo, sizeof(UniformBufferObject));


    // Submit command buffer
    std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore}; // TODO(nic) use Drawable's instead
    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
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

    const VkResult result = vkQueueSubmit(m_renderer.combinedQueue, 1, &submitInfo, inFlightFence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
    }
}

//void Drawable::ClearWindow(VkImage& image)
//{
//    vkResetCommandBuffer(commandBuffer, 0);
//    VkCommandBufferBeginInfo beginInfo;
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.pNext = nullptr;
//    beginInfo.flags = 0;
//    beginInfo.pInheritanceInfo = nullptr;
//    vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//    VkImageSubresourceRange subresourceRange;
//    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    subresourceRange.baseMipLevel = 0;
//    subresourceRange.levelCount = 1;
//    subresourceRange.baseArrayLayer = 0;
//    subresourceRange.layerCount = 1;
//
//    // Move the image into the correct layout
//    // No? just use clearcolorimage
//    VkImageMemoryBarrier generalToClearBarrier;
//    generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    generalToClearBarrier.pNext = nullptr;
//    generalToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//    generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//    generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//    generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    generalToClearBarrier.image = image;
//    generalToClearBarrier.subresourceRange = subresourceRange;
//
//    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
//    // VK_PIPELINE_STAGE_TRANSFER_BIT
//    //  Use this for now to clear the image to a specific color
//    const VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};
//
//    vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);
//
//    VkImageMemoryBarrier clearToPresentBarrier;
//    clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    clearToPresentBarrier.pNext = nullptr;
//    clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//    clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//    clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//    clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    clearToPresentBarrier.image = image;
//    clearToPresentBarrier.subresourceRange = subresourceRange;
//
//    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
//
//    vkEndCommandBuffer(commandBuffer);
//}


//void Drawable::RenderTriangle(uint32_t imageIndex)
//{
//    VkResult result = VK_SUCCESS;
//
//    GraphicsPipelineDescriptor descriptor;
//    const std::filesystem::path shaderDirectory = m_renderer.rendererBase.projectDirectory.parent_path() / "shaders";
//    std::cout << "Shader directory: " << shaderDirectory << "\n";
//    descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
//    descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};
//
//    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
//    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
//
//    descriptor.viewports[0].width = static_cast<float>(m_renderer.display->swapchainExtent.width);
//    descriptor.viewports[0].height = static_cast<float>(m_renderer.display->swapchainExtent.height);
//
//    descriptor.scissors[0].extent = m_renderer.display->swapchainExtent;
//
//    descriptor.colorAttachment.format = m_renderer.display->swapchainImageFormat;
//
//    pipelineDescriptors.emplace_back(descriptor);
//
//    pipelineStates.emplace_back(m_renderer.device, descriptor);
//
//    vkResetCommandBuffer(commandBuffer, 0);
//
//    VkCommandBufferBeginInfo beginInfo;
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.pNext = nullptr;
//    beginInfo.flags = 0;
//    beginInfo.pInheritanceInfo = nullptr;
//    vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//    // std::vector<VkFramebuffer> framebuffers;
//    framebuffers.resize(m_renderer.display->image->imageViews.size());
//    for (size_t i = 0; i < m_renderer.display->image->imageViews.size(); i++)
//    {
//        std::array<VkImageView, 1> attachments = {m_renderer.display->image->imageViews[i]};
//        VkFramebufferCreateInfo framebufferCI;
//        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferCI.pNext = nullptr;
//        framebufferCI.flags = 0;
//        framebufferCI.renderPass = pipelineStates[0].renderPass;
//        framebufferCI.attachmentCount = 1;
//        framebufferCI.pAttachments = attachments.data();
//        framebufferCI.width = m_renderer.display->swapchainExtent.width;
//        framebufferCI.height = m_renderer.display->swapchainExtent.height;
//        framebufferCI.layers = 1;
//        result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[i]);
//        if (result != VK_SUCCESS)
//        {
//            throw std::runtime_error("Failed to create framebuffer");
//        }
//    }
//    // start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
//    VkRenderPassBeginInfo renderPassBI;
//    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//    renderPassBI.pNext = nullptr;
//    renderPassBI.renderPass = pipelineStates[0].renderPass;
//    renderPassBI.framebuffer = framebuffers[imageIndex];
//    renderPassBI.renderArea.offset = {0, 0};
//    renderPassBI.renderArea.extent = m_renderer.display->swapchainExtent;
//    const VkClearValue clearColor = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
//    renderPassBI.clearValueCount = 1;
//    renderPassBI.pClearValues = &clearColor;
//    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
//
//    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].graphicsPipeline);
//
//    vkCmdSetViewport(commandBuffer, 0, 1, descriptor.viewports.data());
//
//    vkCmdSetScissor(commandBuffer, 0, 1, descriptor.scissors.data());
//
//    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
//
//    vkCmdEndRenderPass(commandBuffer);
//
//    vkEndCommandBuffer(commandBuffer);
//}

void Drawable::Render(uint32_t imageIndex)
{
    //VkResult result = VK_SUCCESS;




//    vkWaitForFences(m_renderer.device, 1, &inFlightFence, VK_TRUE, UINT64_MAX); // TODO(nic) use a Drawable owned fence instead
//    vkResetFences(m_renderer.device, 1, &inFlightFence);

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // std::vector<VkFramebuffer> framebuffers;
    //framebuffers.resize(m_renderer.scenes[0].cameras[0].image.imageViews.size());
//    for (size_t i = 0; i < m_renderer.scenes[0].cameras[0].image.imageViews.size(); i++)
//    {
//        std::array<VkImageView, 1> attachments = {m_renderer.scenes[0].cameras[0].image.imageViews[imageIndex]};
//        VkFramebufferCreateInfo framebufferCI;
//        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferCI.pNext = nullptr;
//        framebufferCI.flags = 0;
//        framebufferCI.renderPass = pipelineStates[0].renderPass;
//        framebufferCI.attachmentCount = 1;
//        framebufferCI.pAttachments = attachments.data();
//        framebufferCI.width = m_renderer.display->swapchainExtent.width;
//        framebufferCI.height = m_renderer.display->swapchainExtent.height;
//        framebufferCI.layers = 1;
//        std::cout << "About to create framebuffer\n";
//        result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[imageIndex]);
//        if (result != VK_SUCCESS)
//        {
//            throw std::runtime_error("Failed to create framebuffer");
//        }
//        std::cout << "Created framebuffer\n";
//    }
    // start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
    VkRenderPassBeginInfo renderPassBI;
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.pNext = nullptr;
    renderPassBI.renderPass = pipelineStates[0].renderPass;
    renderPassBI.framebuffer = framebuffers[imageIndex];
    renderPassBI.renderArea.offset = {0, 0};
    renderPassBI.renderArea.extent = m_renderer.display->swapchainExtent;
    std::array<VkClearValue, 2> clearColors;
    clearColors[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    clearColors[1].depthStencil = {1.0F, 0};
    renderPassBI.clearValueCount = clearColors.size();
    renderPassBI.pClearValues = clearColors.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].graphicsPipeline);

    std::array<VkBuffer, 1> vertexBuffers = {vertexBuffer.buffer.buffer};
    std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdSetViewport(commandBuffer, 0, 1, pipelineDescriptors.viewports.data());

    vkCmdSetScissor(commandBuffer, 0, 1, pipelineDescriptors.scissors.data());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].pipelineLayout, 0, 1, &pipelineStates[0].descriptorSets[imageIndex], 0, nullptr);
    //vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer.count), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);
}

void Drawable::render()
{
	recordCommandBuffer();

	executeCommandBuffer();
}

void Drawable::recordCommandBuffer()
{
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
    VkRenderPassBeginInfo renderPassBI;
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.pNext = nullptr;
    renderPassBI.renderPass = pipelineStates[0].renderPass;
    renderPassBI.framebuffer = framebuffers[0];
    renderPassBI.renderArea.offset = {0, 0};
    //renderPassBI.renderArea.extent = m_renderer.display->swapchainExtent;
    renderPassBI.renderArea.extent = m_scene.camera.extent;
    std::array<VkClearValue, 2> clearColors;
    clearColors[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    clearColors[1].depthStencil = {1.0F, 0};
    renderPassBI.clearValueCount = clearColors.size();
    renderPassBI.pClearValues = clearColors.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].graphicsPipeline);

    std::array<VkBuffer, 1> vertexBuffers = {vertexBuffer.buffer.buffer};
    std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    if (!pipelineDescriptors.dynamicStates.empty())
    {
        vkCmdSetViewport(commandBuffer, 0, 1, pipelineDescriptors.viewports.data());

        vkCmdSetScissor(commandBuffer, 0, 1, pipelineDescriptors.scissors.data());
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].pipelineLayout, 0, 1, &pipelineStates[0].descriptorSets[0], 0, nullptr);
    //vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexBuffer.count), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);
}

void Drawable::executeCommandBuffer()
{
	// Update uniform data
	UniformBufferObject ubo;
    ubo.model = transform;
    ubo.view = m_scene.camera.transform;//glm::mat4(1.0F);//glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.proj = m_scene.camera.perspective;//glm::mat4(1.0F);//glm::perspective(glm::radians(45.0F), static_cast<float>(renderer.display->swapchainExtent.width) / static_cast<float>(renderer.display->swapchainExtent.height), 0.1F, 10.0F);
    //ubo.proj = glm::perspective(glm::radians(45.0F), static_cast<float>(800) / static_cast<float>(600), 0.1F, 10.0F);
    ubo.proj[1][1] *= -1;
    std::memcpy(uniformBuffers[0].data, &ubo, sizeof(UniformBufferObject));
    // Submit command buffer
//    std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore}; // TODO(nic) use Drawable's instead
//    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
    std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;//waitSemaphores.size();
    submitInfo.pWaitSemaphores = nullptr;//waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;//signalSemaphores.size();
    submitInfo.pSignalSemaphores = nullptr;//signalSemaphores.data();

    const VkResult result = vkQueueSubmit(m_renderer.combinedQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
    }
    vkQueueWaitIdle(m_renderer.combinedQueue); // TODO (nic) this will have issues with other gpu jobs going
}

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + filename + "\n");
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return buffer;
}

GraphicsPipelineDescriptor::GraphicsPipelineDescriptor() : inputAssembly({}),
                                                           rasterizer({}),
                                                           multisampling({}),
                                                           colorBlending({}),
                                                           colorAttachment({}),
														   depthAttachment({})
{
    // Fill out structures with reasonable defaults for creating a graphics pipeline

    // going to need vertex/index binding things here for VkPipelineVertexInputStateCreateInfo

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = 0.0F;
    viewport.height = 0.0F;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    viewports.emplace_back(viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = {0, 0};
    scissors.emplace_back(scissor);

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//VK_POLYGON_MODE_LINE;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0F;
    rasterizer.depthBiasClamp = 0.0F;
    rasterizer.depthBiasSlopeFactor = 0.0F;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0F;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.pNext = nullptr;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0F;
    depthStencil.maxDepthBounds = 1.0F;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // depth and stencil state to be added later
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachments.emplace_back(colorBlendAttachment);

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0F;
    colorBlending.blendConstants[1] = 0.0F;
    colorBlending.blendConstants[2] = 0.0F;
    colorBlending.blendConstants[3] = 0.0F;

    colorAttachment.flags = 0;
    colorAttachment.format = VK_FORMAT_UNDEFINED;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    depthAttachment.flags = 0;
    depthAttachment.format = VK_FORMAT_UNDEFINED;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

// clang-tidy doesn't understand that Vulkan initializes several of the class members
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
GraphicsPipelineState::GraphicsPipelineState(VkDevice& device, VulkanRenderer& renderer, Drawable& drawable, const GraphicsPipelineDescriptor& descriptor) : m_device(device)
{
    VkResult result = VK_SUCCESS;

    auto vertShaderCode = readFile(descriptor.vertexShader[0]);
    auto fragShaderCode = readFile(descriptor.fragmentShader[0]);

    VkShaderModuleCreateInfo vertShaderCI;
    vertShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderCI.pNext = nullptr;
    vertShaderCI.flags = 0;
    vertShaderCI.codeSize = vertShaderCode.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    vertShaderCI.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
    result = vkCreateShaderModule(device, &vertShaderCI, nullptr, &vertShaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex shader module\n");
    }
    VkPipelineShaderStageCreateInfo vertShaderStageCI;
    vertShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCI.pNext = nullptr;
    vertShaderStageCI.flags = 0;
    vertShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCI.module = vertShaderModule;
    vertShaderStageCI.pName = descriptor.vertexShader[1].c_str();
    vertShaderStageCI.pSpecializationInfo = nullptr;

    VkShaderModuleCreateInfo fragShaderCI;
    fragShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderCI.pNext = nullptr;
    fragShaderCI.flags = 0;
    fragShaderCI.codeSize = fragShaderCode.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    fragShaderCI.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
    result = vkCreateShaderModule(device, &fragShaderCI, nullptr, &fragShaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fragment shader module\n");
    }
    VkPipelineShaderStageCreateInfo fragShaderStageCI;
    fragShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCI.pNext = nullptr;
    fragShaderStageCI.flags = 0;
    fragShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCI.module = fragShaderModule;
    fragShaderStageCI.pName = descriptor.fragmentShader[1].c_str();
    fragShaderStageCI.pSpecializationInfo = nullptr;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageCI, fragShaderStageCI};

    VkVertexInputBindingDescription vertexBinding;
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(Vertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> vertexAttributes {};
    vertexAttributes[0].location = 0;
    vertexAttributes[0].binding = 0;
    vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributes[0].offset = offsetof(Vertex, pos);

    vertexAttributes[1].location = 1;
    vertexAttributes[1].binding = 0;
    vertexAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributes[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.pNext = nullptr;
    vertexInput.flags = 0;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &vertexBinding;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
    vertexInput.pVertexAttributeDescriptions = vertexAttributes.data();

    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(descriptor.dynamicStates.size());
    dynamicState.pDynamicStates = descriptor.dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = static_cast<uint32_t>(descriptor.viewports.size());
    auto viewportDynamicState = std::find(descriptor.dynamicStates.begin(), descriptor.dynamicStates.end(), VK_DYNAMIC_STATE_VIEWPORT);
    viewportState.pViewports = viewportDynamicState == descriptor.dynamicStates.end() ? descriptor.viewports.data() : nullptr;
    viewportState.scissorCount = static_cast<uint32_t>(descriptor.scissors.size());
    auto scissorDynamicState = std::find(descriptor.dynamicStates.begin(), descriptor.dynamicStates.end(), VK_DYNAMIC_STATE_SCISSOR);
    viewportState.pScissors = scissorDynamicState == descriptor.dynamicStates.end() ? descriptor.scissors.data() : nullptr;

    // TODO (nic) move the descriptors someplace else
    VkDescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCi;
    descriptorSetLayoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCi.pNext = nullptr;
    descriptorSetLayoutCi.flags = 0;
    descriptorSetLayoutCi.bindingCount = 1;
    descriptorSetLayoutCi.pBindings = &uboLayoutBinding;
    result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCi, nullptr, &descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
    	throw std::runtime_error("Failed to create descriptor set layout\n");
    }

//    std::vector<VkDescriptorSetLayout> layouts (renderer.display->surfaceCapabilities.minImageCount, descriptorSetLayout);
//    VkDescriptorSetAllocateInfo descriptorSetAi;
//    descriptorSetAi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    descriptorSetAi.pNext = nullptr;
//    descriptorSetAi.descriptorPool = renderer.descriptorPool;
//    descriptorSetAi.descriptorSetCount = static_cast<uint32_t>(layouts.size());
//    descriptorSetAi.pSetLayouts = layouts.data();
//    descriptorSets.resize(layouts.size());
//    result = vkAllocateDescriptorSets(device, &descriptorSetAi, descriptorSets.data());
//    if (result != VK_SUCCESS)
//    {
//    	throw std::runtime_error("Failed to allocate descriptor sets\n");
//    }
    //std::vector<VkDescriptorSetLayout> layouts (renderer.display->surfaceCapabilities.minImageCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAi;
    descriptorSetAi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAi.pNext = nullptr;
    descriptorSetAi.descriptorPool = renderer.descriptorPool;
    descriptorSetAi.descriptorSetCount = 1;
    descriptorSetAi.pSetLayouts = &descriptorSetLayout;
    descriptorSets.resize(1);
    result = vkAllocateDescriptorSets(device, &descriptorSetAi, descriptorSets.data());
    if (result != VK_SUCCESS)
    {
    	throw std::runtime_error("Failed to allocate descriptor sets\n");
    }

    for (uint32_t i = 0; i < descriptorSets.size(); i++)
    {
    	VkDescriptorBufferInfo bufferInfo;
    	bufferInfo.buffer = drawable.uniformBuffers[i].buffer.buffer; // TODO (nic) the naming here is atrocious
    	bufferInfo.offset = 0;
    	bufferInfo.range = sizeof(UniformBufferObject);

    	VkWriteDescriptorSet descriptorWrite;
    	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	descriptorWrite.pNext = nullptr;
    	descriptorWrite.dstSet = descriptorSets[i];
    	descriptorWrite.dstBinding = 0;
    	descriptorWrite.dstArrayElement = 0;
    	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	descriptorWrite.descriptorCount = 1;
    	descriptorWrite.pBufferInfo = &bufferInfo;
    	descriptorWrite.pImageInfo = nullptr;
    	descriptorWrite.pTexelBufferView = nullptr;
    	vkUpdateDescriptorSets(renderer.device, 1, &descriptorWrite, 0 , nullptr);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCI;
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.pNext = nullptr;
    pipelineLayoutCI.flags = 0;
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCI.pushConstantRangeCount = 0;
    pipelineLayoutCI.pPushConstantRanges = nullptr;
    result = vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout\n");
    }

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{}; // zero initialize - this is a big structure
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {descriptor.colorAttachment, descriptor.depthAttachment};
    VkRenderPassCreateInfo renderPassCI;
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.pNext = nullptr;
    renderPassCI.flags = 0;
    renderPassCI.dependencyCount = 1;
    renderPassCI.pDependencies = &dependency;
    renderPassCI.attachmentCount = attachments.size();
    renderPassCI.pAttachments = attachments.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpass;
    result = vkCreateRenderPass(device, &renderPassCI, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass\n");
    }

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = nullptr;
    pipelineCI.flags = 0;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = &vertexInput;
    pipelineCI.pInputAssemblyState = &descriptor.inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &descriptor.rasterizer;
    pipelineCI.pMultisampleState = &descriptor.multisampling;
    pipelineCI.pDepthStencilState = &descriptor.depthStencil;
    pipelineCI.pColorBlendState = &descriptor.colorBlending;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = pipelineLayout;
    pipelineCI.renderPass = renderPass;
    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.basePipelineIndex = -1;
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline\n");
    }
}

GraphicsPipelineState::~GraphicsPipelineState()
{
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, renderPass, nullptr);
    vkDestroyPipeline(m_device, graphicsPipeline, nullptr);
}

GraphicsPipelineState::GraphicsPipelineState(GraphicsPipelineState&& other) noexcept : m_device(other.m_device),
                                                                                       vertShaderModule(other.vertShaderModule),
                                                                                       fragShaderModule(other.fragShaderModule),
																					   descriptorSetLayout(other.descriptorSetLayout),
                                                                                       pipelineLayout(other.pipelineLayout),
                                                                                       renderPass(other.renderPass),
                                                                                       graphicsPipeline(other.graphicsPipeline)
{

    // Null out the object being moved from
    other.vertShaderModule = VK_NULL_HANDLE;
    other.fragShaderModule = VK_NULL_HANDLE;
    other.pipelineLayout = VK_NULL_HANDLE;
    other.renderPass = VK_NULL_HANDLE;
    other.graphicsPipeline = VK_NULL_HANDLE;
}
