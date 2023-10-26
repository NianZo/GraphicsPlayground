/*
 * VulkanImage.cpp
 *
 *  Created on: Feb 8, 2023
 *      Author: nic
 */

#include "VulkanImage.hpp"
#include "VulkanDisplay.hpp"
#include "VulkanRenderer.hpp"

VulkanImage::VulkanImage(VulkanRenderer& rendererIn, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) :
	renderer(rendererIn)
{
	images.resize(1);
	VkImageCreateInfo imageCi;
	imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCi.pNext = nullptr;
	imageCi.flags = 0;
	imageCi.imageType = VK_IMAGE_TYPE_2D;
	imageCi.extent.width = extent.width;
	imageCi.extent.height = extent.height;
	imageCi.extent.depth = 1;
	imageCi.mipLevels = 1;
	imageCi.arrayLayers = 1;
	imageCi.format = format;
	imageCi.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCi.usage = usage;
	imageCi.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult result = vkCreateImage(renderer.device, &imageCi, nullptr, images.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image\n");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(renderer.device, images[0], &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	result = vkAllocateMemory(renderer.device, &allocInfo, nullptr, imageMemory.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory\n");
	}
	vkBindImageMemory(renderer.device, images[0], imageMemory[0], 0);

	imageViews.resize(1);
    VkImageViewCreateInfo imageViewCI;
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.pNext = nullptr;
    imageViewCI.flags = 0;
    imageViewCI.image = images[0];
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.format = format;
    imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;

    result = vkCreateImageView(renderer.device, &imageViewCI, nullptr, imageViews.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get image views for swapchain\n");
    }
}

VulkanImage::VulkanImage(VulkanRenderer& rendererIn) : renderer(rendererIn)
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(renderer.device, renderer.display->swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(renderer.device, renderer.display->swapchain, &imageCount, images.data());

    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo imageViewCI;
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.pNext = nullptr;
        imageViewCI.flags = 0;
        imageViewCI.image = images[i];
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.format = renderer.display->swapchainImageFormat;
        imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;

        const VkResult result = vkCreateImageView(renderer.device, &imageViewCI, nullptr, &imageViews[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to get image views for swapchain\n");
        }
    }
}

VulkanImage::VulkanImage(VulkanImage&& other) noexcept : renderer(other.renderer)
{
    for (VkImage& image : other.images)
    {
        images.push_back(image);
        image = VK_NULL_HANDLE;
    }

    for (VkImageView& imageView : other.imageViews)
    {
        imageViews.push_back(imageView);
        imageView = VK_NULL_HANDLE;
    }
}

VulkanImage::~VulkanImage()
{
    for (VkImageView& imageView : imageViews)
    {
        vkDestroyImageView(renderer.device, imageView, nullptr);
    }

    // Images from swapchain are destroyed when swapchain is destroyed, do not destroy them manually
    //	for (VkImage& image : images)
    //	{
    //		vkDestroyImage(m_display.m_renderer->device, image, nullptr);
    //	}
}

uint32_t VulkanImage::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
