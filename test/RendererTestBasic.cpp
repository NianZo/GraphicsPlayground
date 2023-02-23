/*
 * RendererTestBasic.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: nic
 */

#include "gtest/gtest.h"
#include "VulkanRenderer.hpp"


TEST(RenderTestBasic, CreateRenderer)
{
	{
		RendererBase rendererBase("RenderTestBasic");
		//VulkanRenderer2 renderer("RenderTestBasic");
		EXPECT_NE(rendererBase.physicalDevices.size(), 0);

		// Verify that the structures describing the physical devices are filled out for at least one valid gpu
		// Can't verify for all because I have a 'null'? gpu with vendor id 0 and no features
		bool validGPUFound = false;
		for (const PhysicalDeviceDescriptor& gpuDescriptor : rendererBase.physicalDevices)
		{
			if (gpuDescriptor.properties.vendorID != 0)
			{
				validGPUFound = true;
				EXPECT_EQ(gpuDescriptor.features.geometryShader, VK_TRUE);
				EXPECT_NE(gpuDescriptor.properties.apiVersion, 0);
				EXPECT_NE(gpuDescriptor.memoryProperties.memoryTypeCount, 0);
				EXPECT_NE(gpuDescriptor.memoryProperties.memoryHeapCount, 0);
			}
		}
		EXPECT_TRUE(validGPUFound);
	}
}
