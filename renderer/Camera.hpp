/*
 * Camera.hpp
 *
 *  Created on: Sep 10, 2023
 *      Author: nic
 */

#ifndef RENDERER_CAMERA_HPP_
#define RENDERER_CAMERA_HPP_

#include "VulkanImage.hpp"
#include <glm/glm.hpp>
#include <span>
// #include "VulkanRenderer.hpp"
class VulkanRenderer;
struct R8G8B8A8Texel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// template<uint16_t X, uint16_t Y>
// class ImageData
//{
//	std::array<R8G8B8A8Texel, X>& operator[](uint16_t index);
//
//	std::array<std::array<R8G8B8A8Texel, X>, Y> data;
// };
//
// template<uint16_t X, uint16_t Y>
// std::array<R8G8B8A8Texel, X>& ImageData<X,Y>::operator[](uint16_t index)
//{
//	return data[index];
// }

class ImageData
{

    uint16_t m_width;
    uint16_t m_height;

  public:
    explicit ImageData(uint16_t width, uint16_t height) :
        m_width(width),
        m_height(height),
        data(m_width * m_height)
    {
    }

    std::vector<R8G8B8A8Texel> data;
    //	std::span<R8G8B8A8Texel> operator[](uint16_t index)
    //	{
    //		return std::span<R8G8B8A8Texel>(data.begin() + width * index, width);
    //	}
    R8G8B8A8Texel& index(uint16_t x, uint16_t y)
    {
        return data[static_cast<size_t>(m_width) * y + x];
    }
};
// TODO (nic) for now I am assuming 1 camera per scene and that the camera is always 'drawable'
// I'll deal with more complicated situations later
class Camera
{
  public:
    explicit Camera(VulkanRenderer& rendererIn);
    explicit Camera(VulkanRenderer& rendererIn, uint16_t width, uint16_t height);
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) noexcept = default;
    Camera& operator=(Camera&&) = delete;
    ~Camera() = default;

    void clear(VkClearColorValue clearColor);
    ImageData& cpuData();

    VulkanRenderer& renderer;
    VkExtent2D extent;
    VulkanImage image;
    VulkanImage depthImage;
    ImageData imageDataCpu;
    VkCommandBuffer commandBuffer;
    glm::mat4 transform;
    glm::mat4 perspective;
    // Include a transformation matrix later to set camera position
    // Probably need a reference/shared_ptr/weak_ptr to the scene that the camera is placed in
};

#endif /* RENDERER_CAMERA_HPP_ */
