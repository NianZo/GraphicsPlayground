/*
 * Camera.hpp
 *
 *  Created on: Sep 10, 2023
 *      Author: nic
 */

#ifndef RENDERER_CAMERA_HPP_
#define RENDERER_CAMERA_HPP_

#include "VulkanImage.hpp"
#include <span>
//#include "VulkanRenderer.hpp"
class VulkanRenderer;
struct R8G8B8A8Texel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

//template<uint16_t X, uint16_t Y>
//class ImageData
//{
//	std::array<R8G8B8A8Texel, X>& operator[](uint16_t index);
//
//	std::array<std::array<R8G8B8A8Texel, X>, Y> data;
//};
//
//template<uint16_t X, uint16_t Y>
//std::array<R8G8B8A8Texel, X>& ImageData<X,Y>::operator[](uint16_t index)
//{
//	return data[index];
//}

class ImageData
{
	std::vector<R8G8B8A8Texel> data;
	uint16_t width;
	uint16_t height;
public:
	std::span<R8G8B8A8Texel> operator[](uint16_t index)
	{
		return std::span<R8G8B8A8Texel>(data.begin() + width * index, width);
	}
};
// TODO (nic) for now I am assuming 1 camera per scene and that the camera is always 'drawable'
// I'll deal with more complicated situations later
class Camera
{
public:
    explicit Camera(VulkanRenderer& rendererIn);
    explicit Camera(VulkanRenderer& rendererIn, [[maybe_unused]]uint16_t width, [[maybe_unused]]uint16_t height) :
    		renderer(rendererIn),
			image(renderer)
    {};
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) noexcept = default;
    Camera& operator=(Camera&&) = delete;
    ~Camera() = default;

    void clear() {}
    ImageData& cpuData() {return imageDataCpu;}

    VulkanRenderer& renderer;
	VulkanImage image;
	ImageData imageDataCpu;
	// Include a transformation matrix later to set camera position
	// Probably need a reference/shared_ptr/weak_ptr to the scene that the camera is placed in
};



#endif /* RENDERER_CAMERA_HPP_ */
