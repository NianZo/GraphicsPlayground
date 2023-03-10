/*
 * Drawable.hpp
 *
 *  Created on: Mar 4, 2023
 *      Author: nic
 */

#ifndef NEWRENDERER_DRAWABLE_HPP_
#define NEWRENDERER_DRAWABLE_HPP_

#include <vulkan/vulkan.h>

class VulkanRenderer2;

class Drawable
{
public:
	Drawable(VulkanRenderer2& renderer, VkCommandPool& pool);
	~Drawable();
	void ClearWindow(VkImage& image);
	void ExecuteCommandBuffer();
	void RenderTriangle();

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

private:
	VulkanRenderer2& m_renderer;
	VkCommandBuffer commandBuffer;
};


#endif /* NEWRENDERER_DRAWABLE_HPP_ */
