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

class VulkanRenderer;

class VulkanImage
{
  public:
    VulkanImage(VulkanRenderer& renderer, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    explicit VulkanImage(VulkanRenderer& renderer);
    VulkanImage(const VulkanImage&) = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;
    VulkanImage(VulkanImage&&) noexcept;
    VulkanImage& operator=(VulkanImage&&) = delete;
    ~VulkanImage();
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkDeviceMemory> imageMemory;
    VkFormat m_format;

    VulkanRenderer& renderer;
    bool isDisplayImage;

    static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

  private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    static bool hasStencilComponent(VkFormat format);
};

#endif /* RENDERER_VULKANIMAGE_HPP_ */
