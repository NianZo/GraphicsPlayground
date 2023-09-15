/*
 * Camera.hpp
 *
 *  Created on: Sep 10, 2023
 *      Author: nic
 */

#ifndef RENDERER_CAMERA_HPP_
#define RENDERER_CAMERA_HPP_

#include "VulkanImage.hpp"
//#include "VulkanRenderer.hpp"
class VulkanRenderer;

// TODO (nic) for now I am assuming 1 camera per scene and that the camera is always 'drawable'
// I'll deal with more complicated situations later
class Camera
{
public:
    explicit Camera(VulkanRenderer& rendererIn);
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) noexcept = default;
    Camera& operator=(Camera&&) = delete;
    ~Camera() = default;

    VulkanRenderer& renderer;
	VulkanImage image;
	// Include a transformation matrix later to set camera position
	// Probably need a reference/shared_ptr/weak_ptr to the scene that the camera is placed in
};


#endif /* RENDERER_CAMERA_HPP_ */
