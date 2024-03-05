/*
 * Scene.cpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#include "Scene.hpp"
#include "Drawable.hpp"
#include "VulkanRenderer.hpp"
#include <algorithm>
#include <iostream>

Scene::Scene(VulkanRenderer& rendererIn, uint16_t cameraWidth, uint16_t cameraHeight) :
    renderer(rendererIn),
    camera(renderer, cameraWidth, cameraHeight),
    clearColor({.uint32 = {0, 0, 0, 255}})
{
    // TODO (nic) this is sized based on all the drawables in a scene.
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;
    // poolSize.descriptorCount = display->surfaceCapabilities.minImageCount;

    VkDescriptorPoolCreateInfo poolCi;
    poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCi.pNext = nullptr;
    poolCi.flags = 0;
    poolCi.poolSizeCount = 1;
    poolCi.pPoolSizes = &poolSize;
    poolCi.maxSets = poolSize.descriptorCount;
    VkResult result = vkCreateDescriptorPool(renderer.device, &poolCi, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool\n");
    }
}

Scene::Scene(std::span<GraphicsPipelineDescriptor> drawableDescriptors, VulkanRenderer& rendererIn, uint16_t cameraWidth, uint16_t cameraHeight) :
    renderer(rendererIn),
    camera(renderer, cameraWidth, cameraHeight),
    clearColor({.float32 = {0.0F, 0.0F, 0.0F, 1.0F}})
{
    std::array<VkDescriptorPoolSize, 1> poolSizes = {{{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(drawableDescriptors.size())}}};
    // poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // poolSizes[0].descriptorCount = drawableDescriptors.size();
    const VkDescriptorPoolCreateInfo poolCi = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = poolSizes[0].descriptorCount,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()};
    VkResult result = vkCreateDescriptorPool(renderer.device, &poolCi, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool\n");
    }

    drawables.reserve(drawableDescriptors.size());
    std::ranges::for_each(drawableDescriptors, [this](GraphicsPipelineDescriptor& descriptor)
                          { drawables.emplace_back(*this, descriptor); });
}

Scene::~Scene()
{
    vkDestroyDescriptorPool(renderer.device, descriptorPool, nullptr);
}

Scene::Scene(Scene&& scene) noexcept :
    renderer(scene.renderer),
    camera(std::move(scene.camera)),
    clearColor(scene.clearColor),
    descriptorPool(scene.descriptorPool)
{
    scene.descriptorPool = VK_NULL_HANDLE;
}

bool Scene::render(uint32_t imageIndex)
{
    if (drawables.empty())
    {
        return true;
    }
    // uint32_t imageIndex = 0;
    //    vkAcquireNextImageKHR(renderer.device, renderer.display->swapchain, UINT64_MAX, drawables[0].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // TODO (nic) this is a hack to use the first drawable

    // VkImage& image = display.get()->image.get()->images[imageIndex];
    // drawable.ClearWindow(image);

    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable)
                          { drawable.Render(imageIndex); });

    // drawable.ExecuteCommandBuffer();
    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable)
                          { drawable.ExecuteCommandBuffer(imageIndex); });

    std::array<VkSemaphore, 1> signalSemaphores = {drawables[0].renderFinishedSemaphore}; // TODO (nic) this is a hack to use the first drawable

    std::array<VkSwapchainKHR, 1> swapchains = {renderer.display->swapchain};
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = signalSemaphores.size();
    presentInfo.pWaitSemaphores = signalSemaphores.data();
    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    const VkResult result = vkQueuePresentKHR(renderer.combinedQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // Assuming that we are about to recreate the swapchain
        return false;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present image\n");
    }
    return true;
}

void Scene::render()
{
    camera.clear(clearColor);

    std::ranges::for_each(drawables, [](Drawable& drawable)
                          { drawable.render(); });
}

ImageData& Scene::renderTargetCpuData()
{
    return camera.cpuData();
}
