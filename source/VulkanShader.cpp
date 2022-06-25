/*
 * VulkanShader.cpp
 *
 *  Created on: Apr 17, 2022
 *      Author: nic
 */

#include "VulkanShader.hpp"
#include "VulkanApplication.hpp"
#include "VulkanDevice.hpp"


void VulkanShader::buildShaderModuleWithSPV(uint32_t* vertShaderText, size_t vertexSPVSize, uint32_t* fragShaderText, size_t fragmentSPVSize)
{
	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;

	VkResult result;

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].flags = 0;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";

	VkShaderModuleCreateInfo vertCreateInfo;
	vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertCreateInfo.pNext = nullptr;
	vertCreateInfo.flags = 0;
	vertCreateInfo.codeSize = vertexSPVSize;
	vertCreateInfo.pCode = vertShaderText;
	result = vkCreateShaderModule(deviceObj->device, &vertCreateInfo, nullptr, &shaderStages[0].module);
	assert(result == VK_SUCCESS);

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].flags = 0;
	shaderStages[1].pSpecializationInfo = nullptr;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";

	VkShaderModuleCreateInfo fragCreateInfo;
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragCreateInfo.pNext = nullptr;
	fragCreateInfo.flags = 0;
	fragCreateInfo.codeSize = fragmentSPVSize;
	fragCreateInfo.pCode = fragShaderText;
	result = vkCreateShaderModule(deviceObj->device, &fragCreateInfo, nullptr, &shaderStages[1].module);
	assert(result == VK_SUCCESS);
}

void VulkanShader::destroyShaders()
{
	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;
	vkDestroyShaderModule(deviceObj->device, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(deviceObj->device, shaderStages[1].module, nullptr);
}

//void VulkanShader::buildShader(const char* vertShaderText, const char* fragShaderText)
//{
//	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;
//
//	VkResult result;
//
//	std::vector<unsigned int> vertexSPV;
//	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	shaderStages[0].pNext = nullptr;
//	shaderStages[0].flags = 0;
//	shaderStages[0].pSpecializationInfo = nullptr;
//	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
//	shaderStages[0].pName = "main";
//
//	glslang::InitializeProcess();
//
//
//}


