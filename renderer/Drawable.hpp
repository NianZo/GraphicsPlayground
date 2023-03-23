/*
 * Drawable.hpp
 *
 *  Created on: Mar 4, 2023
 *      Author: nic
 */

#ifndef RENDERER_DRAWABLE_HPP_
#define RENDERER_DRAWABLE_HPP_

#include <array>
#include <fstream>
#include <vector>
#include <vulkan/vulkan.h>
// #include <set>

class VulkanRenderer;

// Fill out from UI / load from file / store to file
struct __attribute__((aligned(128))) GraphicsPipelineDescriptor
{
    GraphicsPipelineDescriptor(); // Fill out structs with defaults, then user doesn't need to fill out sType, pNext, flags, etc.
    // VkPipelineShaderStageCreateInfo vertexShaderStage;
    // VkPipelineShaderStageCreateInfo fragmentShaderStage;
    std::array<std::string, 2> vertexShader; // consider structured binding or a struct for these
    std::array<std::string, 2> fragmentShader;
    std::vector<VkDynamicState> dynamicStates; // A set could be better here, but you can't directly get a pointer to the data
    // going to need vertex/index binding things here for VkPipelineVertexInputStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    // depth and stencil state to be added later
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    // VkPipelineLayoutCreateInfo pipelineLayoutCI; // Define setLayouts and pushConstantRanges instead of this
    VkAttachmentDescription colorAttachment;
};

// Build all objects using info from GraphicsPipelineDescriptor (and renderpass?)
struct __attribute__((aligned(64))) GraphicsPipelineState
{
    GraphicsPipelineState(VkDevice& device, const GraphicsPipelineDescriptor& descriptor);
    ~GraphicsPipelineState();
    GraphicsPipelineState(const GraphicsPipelineState&) = delete;
    GraphicsPipelineState& operator=(const GraphicsPipelineState&) = delete;
    GraphicsPipelineState(GraphicsPipelineState&&) noexcept;
    GraphicsPipelineState& operator=(GraphicsPipelineState&&) = delete; // Can't do this because m_device can't be changed

    VkDevice& m_device;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
};

class Drawable
{
  public:
    Drawable(VulkanRenderer& renderer, VkCommandPool& pool);
    ~Drawable();
    Drawable(const Drawable&) = delete;
    Drawable& operator=(const Drawable&) = delete;
    Drawable(Drawable&&) noexcept;
    Drawable& operator=(Drawable&&) = delete;
    void ClearWindow(VkImage& image);
    void ExecuteCommandBuffer();
    void RenderTriangle(uint32_t imageIndex);

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

  private:
    VulkanRenderer& m_renderer;
    VkCommandBuffer commandBuffer;
    std::vector<GraphicsPipelineDescriptor> pipelineDescriptors;
    std::vector<GraphicsPipelineState> pipelineStates;
};

#endif /* RENDERER_DRAWABLE_HPP_ */
