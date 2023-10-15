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
#include <iostream>

Scene::Scene(VulkanRenderer& rendererIn) : renderer(rendererIn)
{

}

bool Scene::render(uint32_t imageIndex)
{
	std::cout << "Start scene::render()\n";
	if (drawables.empty())
	{
		std::cout << "no drawables, won't render scene\n";
		return true;
	}
    //uint32_t imageIndex = 0;
    std::cout << "number of drawables: " << drawables.size() << "\n";
//    vkAcquireNextImageKHR(renderer.device, renderer.display->swapchain, UINT64_MAX, drawables[0].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // TODO (nic) this is a hack to use the first drawable

    // VkImage& image = display.get()->image.get()->images[imageIndex];
    // drawable.ClearWindow(image);

    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable){drawable.Render(imageIndex);});

    //drawable.ExecuteCommandBuffer();
    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable){drawable.ExecuteCommandBuffer(imageIndex);});

    std::cout << "About to take semaphore from 1st drawable\n";
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
