/*
 * VulkanRenderer.cpp
 *
 *  Created on: Oct 22, 2022
 *      Author: nic
 */

#include "VulkanRenderer.hpp"
#include "Drawable.hpp"
#include "VulkanDisplay.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

void renderLoop(std::stop_token stopToken, VulkanRenderer& renderer);

VulkanRenderer::VulkanRenderer(RendererBase& base, VkSurfaceKHR& surface, uint32_t index, uint32_t width, uint32_t height) : rendererBase(base),
                                                                                                                             gpu(base.physicalDevices[index]),
                                                                                                                             device(VK_NULL_HANDLE),
                                                                                                                             combinedQueueFamily(FindCombinedQueueFamily(surface)),
                                                                                                                             combinedQueue(VK_NULL_HANDLE),
                                                                                                                             commandPool(VK_NULL_HANDLE),
																															 descriptorPool(VK_NULL_HANDLE)
{
    std::array<float, 1> queuePriorities = {0.0F};
    VkDeviceQueueCreateInfo queueCI;
    queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCI.pNext = nullptr;
    queueCI.flags = 0;
    queueCI.queueFamilyIndex = combinedQueueFamily;
    queueCI.queueCount = 1;
    queueCI.pQueuePriorities = queuePriorities.data();

    VkDeviceCreateInfo deviceCI;
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.pNext = nullptr;
    deviceCI.flags = 0;
    deviceCI.queueCreateInfoCount = 1;
    deviceCI.pQueueCreateInfos = &queueCI;
    deviceCI.enabledLayerCount = 0;
    deviceCI.ppEnabledLayerNames = nullptr;
    deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionNames.size());
    deviceCI.ppEnabledExtensionNames = deviceExtensionNames.data();
    VkPhysicalDeviceFeatures features{};
    features.fillModeNonSolid = VK_TRUE;
    deviceCI.pEnabledFeatures = &features;

    VkResult result = vkCreateDevice(gpu.physicalDevice, &deviceCI, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vulkan device\n");
    }

    vkGetDeviceQueue(device, combinedQueueFamily, 0, &combinedQueue);

    VkCommandPoolCreateInfo commandPoolCI;
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.pNext = nullptr;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCI.queueFamilyIndex = combinedQueueFamily;

    result = vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool for combined queue family\n");
    }

    display = std::make_unique<VulkanDisplay>(*this, surface, width, height);
    //cameras.emplace_back(*this);

    // TODO (nic) this is sized based on all the drawables in a scene. A scene should own this.
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = display->surfaceCapabilities.minImageCount;

    VkDescriptorPoolCreateInfo poolCi;
    poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCi.pNext = nullptr;
    poolCi.flags = 0;
    poolCi.poolSizeCount = 1;
    poolCi.pPoolSizes = &poolSize;
    poolCi.maxSets = poolSize.descriptorCount;
    result = vkCreateDescriptorPool(device, &poolCi, nullptr, &descriptorPool);
    if (result != VK_SUCCESS)
    {
    	throw std::runtime_error("Failed to create descriptor pool\n");
    }
    // TODO (nic) std::mutex, std::scoped_lock, std::stop_source, std::stop_token

    renderThread = std::jthread(renderLoop, std::ref(*this));
}

VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) noexcept : rendererBase(other.rendererBase),
                                                                  display(std::move(other.display)),
                                                                  gpu(other.gpu),
                                                                  device(other.device),
                                                                  combinedQueueFamily(other.combinedQueueFamily),
                                                                  combinedQueue(other.combinedQueue),
                                                                  commandPool(other.commandPool),
																  descriptorPool(other.descriptorPool),
																  scenes(std::move(other.scenes))

{
    other.device = VK_NULL_HANDLE;
    other.combinedQueue = VK_NULL_HANDLE;
    other.commandPool = VK_NULL_HANDLE;
    other.descriptorPool = VK_NULL_HANDLE;
}

VulkanRenderer::~VulkanRenderer()
{
	std::cout << "Started Renderer destructor\n";
	renderThread.request_stop();
	renderThread.join();
	std::cout << "Successfully joined renderThread\n";
    // delete display;
    vkDeviceWaitIdle(device);

    scenes.clear();
    display.reset();
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
}

uint32_t VulkanRenderer::FindCombinedQueueFamily(VkSurfaceKHR& surface)
{
    for (uint32_t i = 0; i < gpu.queueFamilyProperties.size(); i++)
    {
        VkBool32 isSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu.physicalDevice, i, surface, &isSupported);
        if (((gpu.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U) && isSupported != 0U)
        {
            return i;
        }
    }
    throw std::runtime_error("No combined queue family found\n");
}

void VulkanRenderer::Render()
{
    //Drawable drawable(*this, commandPool);

//    uint32_t imageIndex = 0;
//    vkAcquireNextImageKHR(device, display->swapchain, UINT64_MAX, drawables[0].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // TODO (nic) this is a hack to use the first drawable
//
//    // VkImage& image = display.get()->image.get()->images[imageIndex];
//    // drawable.ClearWindow(image);
//
//    //drawable.RenderTriangle(imageIndex);
//    //std::for_each(drawables.begin(), drawables.end(), [this](Drawable& drawable){drawable.RenderTriangle(imageIndex);});
//    std::ranges::for_each(drawables, [imageIndex](Drawable& drawable){drawable.Render(imageIndex);});
//
//    //drawable.ExecuteCommandBuffer();
//    std::ranges::for_each(drawables, [](Drawable& drawable){drawable.ExecuteCommandBuffer();});
//
//    std::array<VkSemaphore, 1> signalSemaphores = {drawables[0].renderFinishedSemaphore}; // TODO (nic) this is a hack to use the first drawable
//
//    std::array<VkSwapchainKHR, 1> swapchains = {display->swapchain};
//    VkPresentInfoKHR presentInfo;
//    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    presentInfo.pNext = nullptr;
//    presentInfo.waitSemaphoreCount = signalSemaphores.size();
//    presentInfo.pWaitSemaphores = signalSemaphores.data();
//    presentInfo.swapchainCount = swapchains.size();
//    presentInfo.pSwapchains = swapchains.data();
//    presentInfo.pImageIndices = &imageIndex;
//    presentInfo.pResults = nullptr;
//
//    const VkResult result = vkQueuePresentKHR(combinedQueue, &presentInfo);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to present image\n");
//    }
}

//void VulkanRenderer::Render(const GraphicsPipelineDescriptor& pipelineDescriptor)
//{
//    Drawable drawable(*this, commandPool);
//
//    uint32_t imageIndex = 0;
//    vkAcquireNextImageKHR(device, display->swapchain, UINT64_MAX, drawable.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
//
//    // VkImage& image = display.get()->image.get()->images[imageIndex];
//    // drawable.ClearWindow(image);
//
//    //drawable.RenderTriangle(imageIndex);
//    //drawable.Render(pipelineDescriptor, imageIndex);
//
//    drawable.ExecuteCommandBuffer();
//
//    std::array<VkSemaphore, 1> signalSemaphores = {drawable.renderFinishedSemaphore};
//
//    std::array<VkSwapchainKHR, 1> swapchains = {display->swapchain};
//    VkPresentInfoKHR presentInfo;
//    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//    presentInfo.pNext = nullptr;
//    presentInfo.waitSemaphoreCount = signalSemaphores.size();
//    presentInfo.pWaitSemaphores = signalSemaphores.data();
//    presentInfo.swapchainCount = swapchains.size();
//    presentInfo.pSwapchains = swapchains.data();
//    presentInfo.pImageIndices = &imageIndex;
//    presentInfo.pResults = nullptr;
//
//    const VkResult result = vkQueuePresentKHR(combinedQueue, &presentInfo);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to present image\n");
//    }
//}

void VulkanRenderer::Resize(VkSurfaceKHR surface, [[maybe_unused]] uint32_t width, [[maybe_unused]] uint32_t height)
{
	std::scoped_lock lock(renderMutex);
    vkDeviceWaitIdle(device);
    display.reset();

    display = std::make_unique<VulkanDisplay>(*this, surface, width, height);

//    std::ranges::for_each(scenes, [this](Scene& scene)
//    		{
//    			std::ranges::for_each(scene.cameras, [this](Camera& camera){camera = std::move(Camera(*this));});
//    		});
    std::cout << "Finished resize\n";
}

void renderLoop(std::stop_token stopToken, VulkanRenderer& renderer)
{
	while (!stopToken.stop_requested())
	{
		std::cout << "Render loop " << "\n";
		{
			std::scoped_lock lock(renderer.renderMutex);
			std::cout << "Acquired renderMutex\n";
			if (!renderer.scenes.empty())
			{
			    vkWaitForFences(renderer.device, 1, &renderer.scenes[0].drawables[0].inFlightFence, VK_TRUE, UINT64_MAX); // TODO(nic) use a Drawable owned fence instead
			    vkResetFences(renderer.device, 1, &renderer.scenes[0].drawables[0].inFlightFence); // TODO (nic) the fence shouldn't be owned by the drawable I think
				uint32_t imageIndex;
				vkAcquireNextImageKHR(renderer.device, renderer.display->swapchain, UINT64_MAX, renderer.scenes[0].drawables[0].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex); // TODO (nic) this is a hack to use the first drawable
			    static auto startTime = std::chrono::high_resolution_clock::now();
			    auto currentTime = std::chrono::high_resolution_clock::now();
			    const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			    //UniformBufferObject ubo;
			    UniformBufferObject& ubo = renderer.scenes[0].drawables[0].ubo;
			    ubo.model = glm::rotate(glm::mat4(1.0F), time * glm::radians(90.0F), glm::vec3(0.0F, 0.0F, 1.0F));
			    ubo.view = glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
			    ubo.proj = glm::perspective(glm::radians(45.0F), static_cast<float>(renderer.display->swapchainExtent.width) / static_cast<float>(renderer.display->swapchainExtent.height), 0.1F, 10.0F);
			    ubo.proj[1][1] *= -1;
			    std::memcpy(renderer.scenes[0].drawables[0].uniformBuffers[imageIndex].data, &ubo, sizeof(UniformBufferObject));

				if (!renderer.scenes[0].render(imageIndex))
				{
					// Render could not complete, stop attempting render
					break;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
	std::cout << "Ending renderLoop thread\n";
}
