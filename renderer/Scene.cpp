/*
 * Scene.cpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#include "Drawable.hpp"
#include "Scene.hpp"
#include "VulkanRenderer.hpp"
#include <algorithm>

Scene::Scene(VulkanRenderer& rendererIn) : renderer(rendererIn)
{

}

void Scene::render()
{
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(renderer.device, renderer.display->swapchain, UINT64_MAX, drawables[0].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // TODO (nic) this is a hack to use the first drawable

    // VkImage& image = display.get()->image.get()->images[imageIndex];
    // drawable.ClearWindow(image);

    //drawable.RenderTriangle(imageIndex);
    //std::for_each(drawables.begin(), drawables.end(), [this](Drawable& drawable){drawable.RenderTriangle(imageIndex);});
    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable){drawable.Render(imageIndex);});

    //drawable.ExecuteCommandBuffer();
    std::ranges::for_each(drawables, [](Drawable& drawable){drawable.ExecuteCommandBuffer();});

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
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present image\n");
    }
}
