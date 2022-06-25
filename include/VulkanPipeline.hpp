/*
 * VulkanPipeline.hpp
 *
 *  Created on: Apr 17, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANPIPELINE_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANPIPELINE_HPP_

#include "Header.hpp"
class VulkanShader;
class VulkanDrawable;
class VulkanDevice;
class VulkanApplication;

#define NUMBER_OF_VIEWPORTS 1
#define NUMBER_OF_SCISSORS NUMBER_OF_VIEWPORTS

class VulkanPipeline
{
public:
	VulkanPipeline();
	~VulkanPipeline() = default;

	void createPipelineCache();
	bool createPipeline(VulkanDrawable* drawableObj, VkPipeline* pipeline, VulkanShader* shaderObj, VkBool32 includeDepth, VkBool32 includeVi = true);
	void destroyPipelineCache();

	VkPipelineCache pipelineCache;
	VkPipelineLayout pipelineLayout;
	VulkanApplication* appObj;
	VulkanDevice* deviceObj;
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANPIPELINE_HPP_ */
