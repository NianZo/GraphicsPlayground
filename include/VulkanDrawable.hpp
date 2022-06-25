/*
 * VulkanDrawable.hpp
 *
 *  Created on: Apr 16, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_VULKANDRAWABLE_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_VULKANDRAWABLE_HPP_

#include "Header.hpp"
#include "VulkanDescriptor.hpp"
#include "Wrapper.hpp"

class VulkanRenderer;

class VulkanDrawable : public VulkanDescriptor
{
public:
	VulkanDrawable(VulkanRenderer* parent = nullptr);
	~VulkanDrawable() override {};

	void createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);
	void createIndexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride);
	void createUniformBuffer();
	void destroyVertexBuffer();
	void destroyIndexBuffer();
	void destroyUniformBuffer();

	// Prepares the drawing object befoer rendering
	// Allocate, create, record command buffer
	void prepare();
	void update();
	void render();
	void initViewports(VkCommandBuffer* cmd);
	void initScissors(VkCommandBuffer* cmd);

	void setPipeline(VkPipeline* vulkanPipeline) {pipeline = vulkanPipeline;}
	VkPipeline* getPipeline() {return pipeline;}

	void createDescriptorLayout(bool useTexture) override;
	void createPipelineLayout() override;
	void createDescriptorPool(bool useTexture) override;
	void createDescriptorResources() override;
	void createDescriptorSet(bool useTexture) override;

	void setTextures(TextureData* tex);

	// Structure storing vertex buffer metadata
	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
	} VertexBuffer;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
	} IndexBuffer;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
		VkMemoryRequirements memRequirement;
		std::vector<VkMappedMemoryRange> mappedRange;
		uint8_t* pData;
	} UniformBuffer;

	// Stores the vertex input rate
	VkVertexInputBindingDescription viIpBind;
	// Store metadata helpful in data interpretation
	VkVertexInputAttributeDescription viIpAttrib[2];
	VulkanRenderer* rendererObj;

private:
	// Command buffer for drawing
	std::vector<VkCommandBuffer> vecCmdDraw;

	VkViewport viewport;
	VkRect2D scissor;
	VkSemaphore presentCompleteSemaphore;
	VkSemaphore drawingCompleteSemaphore;

	TextureData* textures;

	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 MVP;

	// Prepares render pass instance
	void recordCommandBuffer(int currentImage, VkCommandBuffer* cmdDraw);

	VkPipeline* pipeline;
};

#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_VULKANDRAWABLE_HPP_ */
