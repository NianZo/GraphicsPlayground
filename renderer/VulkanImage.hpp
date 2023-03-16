/*
 * VulkanImage.hpp
 *
 *  Created on: Feb 8, 2023
 *      Author: nic
 */

#ifndef RENDERER_VULKANIMAGE_HPP_
#define RENDERER_VULKANIMAGE_HPP_

#include <vector>
#include <vulkan/vulkan.h>

class VulkanDisplay;

class VulkanImage
{
  public:
    explicit VulkanImage(VulkanDisplay& display);
    VulkanImage(const VulkanImage&) = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    VulkanImage(VulkanImage&&) noexcept;
    VulkanImage& operator=(VulkanImage&&) = delete;
    ~VulkanImage();
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VulkanDisplay& m_display;
};

#endif /* RENDERER_VULKANIMAGE_HPP_ */
