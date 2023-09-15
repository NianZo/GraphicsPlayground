/*
 * VulkanImage.cpp
 *
 *  Created on: Feb 8, 2023
 *      Author: nic
 */

#include "VulkanImage.hpp"
#include "VulkanDisplay.hpp"
#include "VulkanRenderer.hpp"

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
