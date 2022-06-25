/*
 * VulkanDescriptor.cpp
 *
 *  Created on: Apr 26, 2022
 *      Author: nic
 */

#include "VulkanDescriptor.hpp"
#include "VulkanDevice.hpp"
#include "VulkanApplication.hpp"
#include <iostream>

VulkanDescriptor::VulkanDescriptor()
{
	deviceObj = VulkanApplication::GetInstance()->deviceObj;
}

void VulkanDescriptor::destroyPipelineLayout()
{
	vkDestroyPipelineLayout(deviceObj->device, pipelineLayout, nullptr);
}

void VulkanDescriptor::destroyDescriptorPool()
{
	vkDestroyDescriptorPool(deviceObj->device, descriptorPool, nullptr);
}

void VulkanDescriptor::destroyDescriptorSet()
{
	vkFreeDescriptorSets(deviceObj->device, descriptorPool, (uint32_t)descriptorSets.size(), &descriptorSets[0]);
}

void VulkanDescriptor::createDescriptor(bool useTexture)
{
	createDescriptorResources();
	std::cout << "created descriptor resources" << std::endl;
	createDescriptorPool(useTexture);
	std::cout << "created descriptor pool" << std::endl;
	createDescriptorSet(useTexture);
	std::cout << "created descriptor set" << std::endl;
}
