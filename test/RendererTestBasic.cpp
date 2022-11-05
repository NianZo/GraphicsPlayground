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
		VulkanRenderer2("RenderTestBasic");
	}
}
