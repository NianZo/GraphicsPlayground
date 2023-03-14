/*
 * VulkanImage.hpp
 *
 *  Created on: Feb 8, 2023
 *      Author: nic
 */

#ifndef RENDERER_VULKANIMAGE_HPP_
#define RENDERER_VULKANIMAGE_HPP_

#include <vulkan/vulkan.h>
#include <vector>

class VulkanDisplay;

class VulkanImage
{
public:
	VulkanImage(VulkanDisplay& display);
	~VulkanImage();
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	VulkanDisplay& m_display;
};


#endif /* RENDERER_VULKANIMAGE_HPP_ */
