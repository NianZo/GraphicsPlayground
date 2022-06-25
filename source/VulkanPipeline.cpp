/*
 * VulkanPipeline.cpp
 *
 *  Created on: Apr 17, 2022
 *      Author: nic
 */

#include "VulkanPipeline.hpp"
#include "VulkanApplication.hpp"
#include "VulkanShader.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanDevice.hpp"
#include <iostream>

VulkanPipeline::VulkanPipeline()
{
	appObj = VulkanApplication::GetInstance();
	deviceObj = appObj->deviceObj;
}

void VulkanPipeline::createPipelineCache()
{
	VkResult result;

	VkPipelineCacheCreateInfo cacheInfo;
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	cacheInfo.pNext = nullptr;
	cacheInfo.flags = 0;
	cacheInfo.initialDataSize = 0;
	cacheInfo.pInitialData = nullptr;
	result = vkCreatePipelineCache(deviceObj->device, &cacheInfo, nullptr, &pipelineCache);
	assert(result == VK_SUCCESS);
}

bool VulkanPipeline::createPipeline(VulkanDrawable* drawableObj, VkPipeline* pipeline, VulkanShader* shaderObj, VkBool32 includeDepth, VkBool32 includeVi)
{
	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	VkDynamicState dynamicStateEnables[2];
	dynamicStateEnables[0] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStateEnables[1] = VK_DYNAMIC_STATE_SCISSOR;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStateEnables;

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
	vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateInfo.pNext = nullptr;
	vertexInputStateInfo.flags = 0;
	if (includeVi)
	{
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &drawableObj->viIpBind;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 2;
		vertexInputStateInfo.pVertexAttributeDescriptions = drawableObj->viIpAttrib;
	} else
	{
		vertexInputStateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;
	}


	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterStateInfo;
	rasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterStateInfo.pNext = nullptr;
	rasterStateInfo.flags = 0;
	rasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterStateInfo.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT; // Changed for testing
	rasterStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterStateInfo.depthClampEnable = VK_FALSE; //includeDepth; // This device feature is disabled
	rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterStateInfo.depthBiasEnable = VK_FALSE;
	rasterStateInfo.depthBiasConstantFactor = 0;
	rasterStateInfo.depthBiasClamp = 0;
	rasterStateInfo.depthBiasSlopeFactor = 0;
	rasterStateInfo.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentStateInfo[1];
	colorBlendAttachmentStateInfo[0].colorWriteMask = 0xf;
	colorBlendAttachmentStateInfo[0].blendEnable = VK_FALSE;
	colorBlendAttachmentStateInfo[0].alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentStateInfo[0].colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentStateInfo[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo;
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.pNext = nullptr;
	colorBlendStateInfo.flags = 0;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = colorBlendAttachmentStateInfo;
	colorBlendStateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateInfo.blendConstants[0] = 1.0f;
	colorBlendStateInfo.blendConstants[1] = 1.0f;
	colorBlendStateInfo.blendConstants[2] = 1.0f;
	colorBlendStateInfo.blendConstants[3] = 1.0f;

	VkPipelineViewportStateCreateInfo viewportStateInfo;
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.pNext = nullptr;
	viewportStateInfo.flags = 0;
	viewportStateInfo.viewportCount = NUMBER_OF_VIEWPORTS;
	viewportStateInfo.scissorCount = NUMBER_OF_SCISSORS;
	viewportStateInfo.pViewports = nullptr; // dynamic state
	viewportStateInfo.pScissors = nullptr; // dynamic state

	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo;
	depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateInfo.pNext = nullptr;
	depthStencilStateInfo.flags = 0;
	depthStencilStateInfo.depthTestEnable = includeDepth;
	depthStencilStateInfo.depthWriteEnable = includeDepth;
	depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStateInfo.back.compareMask = 0;
	depthStencilStateInfo.back.reference = 0;
	depthStencilStateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilStateInfo.back.writeMask = 0;
	depthStencilStateInfo.front = depthStencilStateInfo.back;
	depthStencilStateInfo.minDepthBounds = 0;
	depthStencilStateInfo.maxDepthBounds = 0;

	VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
	multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateInfo.pNext = nullptr;
	multisampleStateInfo.flags = 0;
	multisampleStateInfo.rasterizationSamples = NUM_SAMPLES;
	multisampleStateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateInfo.minSampleShading = 0.0f;
	multisampleStateInfo.pSampleMask = nullptr;
	multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateInfo.alphaToOneEnable = VK_FALSE;

//	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
//	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipelineLayoutCreateInfo.pNext = nullptr;
//	pipelineLayoutCreateInfo.flags = 0;
//	pipelineLayoutCreateInfo.setLayoutCount = 0;
//	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
//	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
//	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
//	vkCreatePipelineLayout(deviceObj->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	// Actual pipeline creation
	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderObj->shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputStateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterStateInfo;
	pipelineInfo.pMultisampleState = &multisampleStateInfo;
	pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
	pipelineInfo.pColorBlendState = &colorBlendStateInfo;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.renderPass = appObj->rendererObj->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.layout = drawableObj->pipelineLayout;
	pipelineInfo.basePipelineHandle = 0;
	pipelineInfo.basePipelineIndex = 0;
	std::cout << "Creating pipeline with this pipelineLayout: " << &drawableObj->pipelineLayout << std::endl;

	return (vkCreateGraphicsPipelines(deviceObj->device, pipelineCache, 1, &pipelineInfo, nullptr, pipeline) == VK_SUCCESS);
}

void VulkanPipeline::destroyPipelineCache()
{
	vkDestroyPipelineCache(deviceObj->device, pipelineCache, nullptr);
	vkDestroyPipelineLayout(deviceObj->device, pipelineLayout, nullptr);
}





































