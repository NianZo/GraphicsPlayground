/*
 * VulkanShader.hpp
 *
 *  Created on: Apr 17, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSHADER_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSHADER_HPP_

#include "Header.hpp"
//#include "SPIRV/GlslangToSpv.h"

class VulkanShader
{
public:
	VulkanShader() = default;
	~VulkanShader() = default;

	// Use .spv to build shader module
	void buildShaderModuleWithSPV(uint32_t* vertShaderText, size_t vertexSPVSize, uint32_t* fragShaderText, size_t framgnetSPVSize);

	void destroyShaders();

	// I don't really care about supporting online compilation right now
//	// Entry point to build the shaders
//	void buildShader(const char* vertShaderText, const char* fragShaderText);
//	// Convert GLSL shader to SPIR-V shader
//	bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char* pshader, std::vector<unsigned int>& spirv);
//	// Kill the shader when not required
//	void destroyShaders();
//
//	// Type of shader language. This could be - EShLangVertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment, and Compute
//	EShLanguage getLanguage(const VkShaderStageFlagBits shaderType);
//
//	// Initialize the TBuiltInResource
//	void initializeResources(TBuiltInResource& resources);

	// Vk structure storing vertex and fragment shader information
	VkPipelineShaderStageCreateInfo shaderStages[2];
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANSHADER_HPP_ */
