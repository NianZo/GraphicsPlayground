/*
 * Camera.cpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#include "Camera.hpp"
//#include <utility>

Camera::Camera(VulkanRenderer& rendererIn) : renderer(rendererIn), image(renderer)
{
	//image(renderer);
}

//Camera::Camera(Camera&& other) : renderer(other.renderer), image(std::move(other.image))
//{
//
//}
