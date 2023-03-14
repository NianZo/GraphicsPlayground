/*
 * VulkanImage.cpp
 *
 *  Created on: Feb 8, 2023
 *      Author: nic
 */

#include "VulkanImage.hpp"

#include "VulkanDisplay.hpp"
#include "VulkanRenderer.hpp"

VulkanImage::VulkanImage(VulkanDisplay& display) : m_display(display)
{
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(display.m_renderer->device, display.swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(display.m_renderer->device, display.swapchain, &imageCount, images.data());

	imageViews.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo imageViewCI;
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCI.pNext = nullptr;
		imageViewCI.flags = 0;
		imageViewCI.image = images[i];
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = display.swapchainImageFormat;
		imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(display.m_renderer->device, &imageViewCI, nullptr, &imageViews[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to get image views for swapchain\n");
		}
	}
}

VulkanImage::~VulkanImage()
{
	for (VkImageView& imageView : imageViews)
	{
		vkDestroyImageView(m_display.m_renderer->device, imageView, nullptr);
	}

	// Images from swapchain are destroyed when swapchain is destroyed, do not destroy them manually
//	for (VkImage& image : images)
//	{
//		vkDestroyImage(m_display.m_renderer->device, image, nullptr);
//	}
}
