/*
 * Drawable.cpp
 *
 *  Created on: Mar 8, 2023
 *      Author: nic
 */

#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

std::vector<char> readFile(const std::string& filename);

// clang-tidy doesn't understand that Vulkan initializes several of the class members
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
Drawable::Drawable(VulkanRenderer& renderer, VkCommandPool& commandPool) : m_renderer(renderer)
{
    VkCommandBufferAllocateInfo commandBufferAI;
    commandBufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAI.pNext = nullptr;
    commandBufferAI.commandPool = commandPool;
    commandBufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAI.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_renderer.device, &commandBufferAI, &commandBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffer from combined queue family command pool\n");
    }

    VkSemaphoreCreateInfo semaphoreCI;
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = nullptr;
    semaphoreCI.flags = 0;

    result = vkCreateSemaphore(m_renderer.device, &semaphoreCI, nullptr, &imageAvailableSemaphore);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vkSemaphore\n");
    }
    result = vkCreateSemaphore(m_renderer.device, &semaphoreCI, nullptr, &renderFinishedSemaphore);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vkSemaphore\n");
    }

    VkFenceCreateInfo fenceCI;
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(m_renderer.device, &fenceCI, nullptr, &inFlightFence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fence\n");
    }
}

Drawable::~Drawable()
{
    vkDeviceWaitIdle(m_renderer.device);
    vkDestroySemaphore(m_renderer.device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_renderer.device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(m_renderer.device, inFlightFence, nullptr);
    for (VkFramebuffer& framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(m_renderer.device, framebuffer, nullptr);
    }
}

void Drawable::ExecuteCommandBuffer()
{
    // Should the queue execution happen in renderer instead of drawable?
    // Drawable should own fences and semaphores, but renderer owns the queues that this will be submitted to
    vkWaitForFences(m_renderer.device, 1, &inFlightFence, VK_TRUE, UINT64_MAX); // TODO(nic) use a Drawable owned fence instead
    vkResetFences(m_renderer.device, 1, &inFlightFence);

    // Submit command buffer
    std::array<VkSemaphore, 1> waitSemaphores = {imageAvailableSemaphore}; // TODO(nic) use Drawable's instead
    std::array<VkSemaphore, 1> signalSemaphores = {renderFinishedSemaphore};
    std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    const VkResult result = vkQueueSubmit(m_renderer.combinedQueue, 1, &submitInfo, inFlightFence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit drawing buffer: \n" + std::to_string(result));
    }
}

void Drawable::ClearWindow(VkImage& image)
{
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    // Move the image into the correct layout
    // No? just use clearcolorimage
    VkImageMemoryBarrier generalToClearBarrier;
    generalToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    generalToClearBarrier.pNext = nullptr;
    generalToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    generalToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    generalToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    generalToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    generalToClearBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    generalToClearBarrier.image = image;
    generalToClearBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &generalToClearBarrier);
    // VK_PIPELINE_STAGE_TRANSFER_BIT
    //  Use this for now to clear the image to a specific color
    const VkClearColorValue clearColor = {{0.42F, 1.0F, 0.46F, 1.0F}};

    vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &subresourceRange);

    VkImageMemoryBarrier clearToPresentBarrier;
    clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    clearToPresentBarrier.pNext = nullptr;
    clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    clearToPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    clearToPresentBarrier.image = image;
    clearToPresentBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

    vkEndCommandBuffer(commandBuffer);
}
void Drawable::RenderTriangle(uint32_t imageIndex)
{
    VkResult result = VK_SUCCESS;

    GraphicsPipelineDescriptor descriptor;
    const std::filesystem::path shaderDirectory = m_renderer.rendererBase.projectDirectory.parent_path() / "shaders";
    std::cout << "Shader directory: " << shaderDirectory << "\n";
    descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
    descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};

    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

    descriptor.viewports[0].width = static_cast<float>(m_renderer.display->swapchainExtent.width);
    descriptor.viewports[0].height = static_cast<float>(m_renderer.display->swapchainExtent.height);

    descriptor.scissors[0].extent = m_renderer.display->swapchainExtent;

    descriptor.colorAttachment.format = m_renderer.display->swapchainImageFormat;

    pipelineDescriptors.emplace_back(descriptor);

    pipelineStates.emplace_back(m_renderer.device, descriptor);

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(m_renderer.display->image->imageViews.size());
    for (size_t i = 0; i < m_renderer.display->image->imageViews.size(); i++)
    {
        std::array<VkImageView, 1> attachments = {m_renderer.display->image->imageViews[i]};
        VkFramebufferCreateInfo framebufferCI;
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.pNext = nullptr;
        framebufferCI.flags = 0;
        framebufferCI.renderPass = pipelineStates[0].renderPass;
        framebufferCI.attachmentCount = 1;
        framebufferCI.pAttachments = attachments.data();
        framebufferCI.width = m_renderer.display->swapchainExtent.width;
        framebufferCI.height = m_renderer.display->swapchainExtent.height;
        framebufferCI.layers = 1;
        result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
    // start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
    VkRenderPassBeginInfo renderPassBI;
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.pNext = nullptr;
    renderPassBI.renderPass = pipelineStates[0].renderPass;
    renderPassBI.framebuffer = framebuffers[imageIndex];
    renderPassBI.renderArea.offset = {0, 0};
    renderPassBI.renderArea.extent = m_renderer.display->swapchainExtent;
    const VkClearValue clearColor = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    renderPassBI.clearValueCount = 1;
    renderPassBI.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].graphicsPipeline);

    vkCmdSetViewport(commandBuffer, 0, 1, descriptor.viewports.data());

    vkCmdSetScissor(commandBuffer, 0, 1, descriptor.scissors.data());

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);
}

void Drawable::Render(const GraphicsPipelineDescriptor& pipelineDescriptor, uint32_t imageIndex)
{
    VkResult result = VK_SUCCESS;

    GraphicsPipelineDescriptor descriptor = pipelineDescriptor;
    //descriptor.rasterizer.polygonMode = pipelineDescriptor.rasterizer.polygonMode;
    const std::filesystem::path shaderDirectory = m_renderer.rendererBase.projectDirectory.parent_path() / "shaders";
    if (descriptor.vertexShader[0] == "")
    {
        std::cout << "Shader directory: " << shaderDirectory << "\n";
        descriptor.vertexShader = {std::string(shaderDirectory / "DrawTriangle-vert.spv"), "main"};
    }

    if (descriptor.fragmentShader[0] == "")
    {
    	descriptor.fragmentShader = {std::string(shaderDirectory / "DrawTriangle-frag.spv"), "main"};
    }

    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    descriptor.dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

    descriptor.viewports[0].width = static_cast<float>(m_renderer.display->swapchainExtent.width);
    descriptor.viewports[0].height = static_cast<float>(m_renderer.display->swapchainExtent.height);

    descriptor.scissors[0].extent = m_renderer.display->swapchainExtent;

    descriptor.colorAttachment.format = m_renderer.display->swapchainImageFormat;

    pipelineDescriptors.emplace_back(descriptor);

    pipelineStates.emplace_back(m_renderer.device, descriptor);

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(m_renderer.display->image->imageViews.size());
    for (size_t i = 0; i < m_renderer.display->image->imageViews.size(); i++)
    {
        std::array<VkImageView, 1> attachments = {m_renderer.display->image->imageViews[i]};
        VkFramebufferCreateInfo framebufferCI;
        framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCI.pNext = nullptr;
        framebufferCI.flags = 0;
        framebufferCI.renderPass = pipelineStates[0].renderPass;
        framebufferCI.attachmentCount = 1;
        framebufferCI.pAttachments = attachments.data();
        framebufferCI.width = m_renderer.display->swapchainExtent.width;
        framebufferCI.height = m_renderer.display->swapchainExtent.height;
        framebufferCI.layers = 1;
        result = vkCreateFramebuffer(m_renderer.device, &framebufferCI, nullptr, &framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
    // start renderpass, bind pipeline, set viewport/scissor, draw, end renderpass
    VkRenderPassBeginInfo renderPassBI;
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.pNext = nullptr;
    renderPassBI.renderPass = pipelineStates[0].renderPass;
    renderPassBI.framebuffer = framebuffers[imageIndex];
    renderPassBI.renderArea.offset = {0, 0};
    renderPassBI.renderArea.extent = m_renderer.display->swapchainExtent;
    const VkClearValue clearColor = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    renderPassBI.clearValueCount = 1;
    renderPassBI.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineStates[0].graphicsPipeline);

    vkCmdSetViewport(commandBuffer, 0, 1, descriptor.viewports.data());

    vkCmdSetScissor(commandBuffer, 0, 1, descriptor.scissors.data());

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);
}

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + filename + "\n");
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    return buffer;
}

GraphicsPipelineDescriptor::GraphicsPipelineDescriptor() : inputAssembly({}),
                                                           rasterizer({}),
                                                           multisampling({}),
                                                           colorBlending({}),
                                                           colorAttachment({})
{
    // Fill out structures with reasonable defaults for creating a graphics pipeline

    // going to need vertex/index binding things here for VkPipelineVertexInputStateCreateInfo

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = 0.0F;
    viewport.height = 0.0F;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    viewports.emplace_back(viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = {0, 0};
    scissors.emplace_back(scissor);

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;//VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0F;
    rasterizer.depthBiasClamp = 0.0F;
    rasterizer.depthBiasSlopeFactor = 0.0F;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.flags = 0;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0F;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // depth and stencil state to be added later
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachments.emplace_back(colorBlendAttachment);

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0F;
    colorBlending.blendConstants[1] = 0.0F;
    colorBlending.blendConstants[2] = 0.0F;
    colorBlending.blendConstants[3] = 0.0F;

    colorAttachment.flags = 0;
    colorAttachment.format = VK_FORMAT_UNDEFINED;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

// clang-tidy doesn't understand that Vulkan initializes several of the class members
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
GraphicsPipelineState::GraphicsPipelineState(VkDevice& device, const GraphicsPipelineDescriptor& descriptor) : m_device(device)
{
    std::cout << "In GraphicsPipelineState constructor\n";
    VkResult result = VK_SUCCESS;

    auto vertShaderCode = readFile(descriptor.vertexShader[0]);
    auto fragShaderCode = readFile(descriptor.fragmentShader[0]);

    VkShaderModuleCreateInfo vertShaderCI;
    vertShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderCI.pNext = nullptr;
    vertShaderCI.flags = 0;
    vertShaderCI.codeSize = vertShaderCode.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    vertShaderCI.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
    result = vkCreateShaderModule(device, &vertShaderCI, nullptr, &vertShaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex shader module\n");
    }
    VkPipelineShaderStageCreateInfo vertShaderStageCI;
    vertShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCI.pNext = nullptr;
    vertShaderStageCI.flags = 0;
    vertShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCI.module = vertShaderModule;
    vertShaderStageCI.pName = descriptor.vertexShader[1].c_str();
    vertShaderStageCI.pSpecializationInfo = nullptr;

    VkShaderModuleCreateInfo fragShaderCI;
    fragShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderCI.pNext = nullptr;
    fragShaderCI.flags = 0;
    fragShaderCI.codeSize = fragShaderCode.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    fragShaderCI.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
    result = vkCreateShaderModule(device, &fragShaderCI, nullptr, &fragShaderModule);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create fragment shader module\n");
    }
    VkPipelineShaderStageCreateInfo fragShaderStageCI;
    fragShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCI.pNext = nullptr;
    fragShaderStageCI.flags = 0;
    fragShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCI.module = fragShaderModule;
    fragShaderStageCI.pName = descriptor.fragmentShader[1].c_str();
    fragShaderStageCI.pSpecializationInfo = nullptr;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageCI, fragShaderStageCI};

    VkPipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.pNext = nullptr;
    vertexInput.flags = 0;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.pVertexBindingDescriptions = nullptr;
    vertexInput.vertexAttributeDescriptionCount = 0;
    vertexInput.pVertexAttributeDescriptions = nullptr;

    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(descriptor.dynamicStates.size());
    dynamicState.pDynamicStates = descriptor.dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = static_cast<uint32_t>(descriptor.viewports.size());
    auto viewportDynamicState = std::find(descriptor.dynamicStates.begin(), descriptor.dynamicStates.end(), VK_DYNAMIC_STATE_VIEWPORT);
    viewportState.pViewports = viewportDynamicState == descriptor.dynamicStates.end() ? nullptr : descriptor.viewports.data();
    viewportState.scissorCount = static_cast<uint32_t>(descriptor.scissors.size());
    auto scissorDynamicState = std::find(descriptor.dynamicStates.begin(), descriptor.dynamicStates.end(), VK_DYNAMIC_STATE_SCISSOR);
    viewportState.pScissors = scissorDynamicState == descriptor.dynamicStates.end() ? nullptr : descriptor.scissors.data();

    VkPipelineLayoutCreateInfo pipelineLayoutCI;
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.pNext = nullptr;
    pipelineLayoutCI.flags = 0;
    pipelineLayoutCI.setLayoutCount = 0;
    pipelineLayoutCI.pSetLayouts = nullptr;
    pipelineLayoutCI.pushConstantRangeCount = 0;
    pipelineLayoutCI.pPushConstantRanges = nullptr;
    result = vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout\n");
    }

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{}; // zero initialize - this is a big structure
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassCI;
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.pNext = nullptr;
    renderPassCI.flags = 0;
    renderPassCI.dependencyCount = 0;
    renderPassCI.pDependencies = nullptr;
    renderPassCI.attachmentCount = 1;
    renderPassCI.pAttachments = &descriptor.colorAttachment;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpass;
    result = vkCreateRenderPass(device, &renderPassCI, nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass\n");
    }

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = nullptr;
    pipelineCI.flags = 0;
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = &vertexInput;
    pipelineCI.pInputAssemblyState = &descriptor.inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &descriptor.rasterizer;
    pipelineCI.pMultisampleState = &descriptor.multisampling;
    pipelineCI.pDepthStencilState = nullptr;
    pipelineCI.pColorBlendState = &descriptor.colorBlending;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = pipelineLayout;
    pipelineCI.renderPass = renderPass;
    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.basePipelineIndex = -1;
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline\n");
    }
}

GraphicsPipelineState::~GraphicsPipelineState()
{
    std::cout << "In GraphicsPipelineState destructor\n";
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, renderPass, nullptr);
    vkDestroyPipeline(m_device, graphicsPipeline, nullptr);
}

GraphicsPipelineState::GraphicsPipelineState(GraphicsPipelineState&& other) noexcept : m_device(other.m_device),
                                                                                       vertShaderModule(other.vertShaderModule),
                                                                                       fragShaderModule(other.fragShaderModule),
                                                                                       pipelineLayout(other.pipelineLayout),
                                                                                       renderPass(other.renderPass),
                                                                                       graphicsPipeline(other.graphicsPipeline)
{
    std::cout << "In GraphicsPipelineState move constructor\n";

    // Null out the object being moved from
    other.vertShaderModule = VK_NULL_HANDLE;
    other.fragShaderModule = VK_NULL_HANDLE;
    other.pipelineLayout = VK_NULL_HANDLE;
    other.renderPass = VK_NULL_HANDLE;
    other.graphicsPipeline = VK_NULL_HANDLE;
}
