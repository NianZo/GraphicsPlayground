/*
 * Scene.hpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#ifndef RENDERER_SCENE_HPP_
#define RENDERER_SCENE_HPP_

#include "Camera.hpp"
#include <vector>

class Drawable;
class VulkanRenderer;
class ImageData;

class Scene
{
  public:
    explicit Scene(VulkanRenderer& renderer, uint16_t cameraWidth, uint16_t cameraHeight);
    explicit Scene(std::span<GraphicsPipelineDescriptor> drawableDescriptors, VulkanRenderer& renderer, uint16_t cameraWidth, uint16_t cameraHeight);
    ~Scene();
    Scene(Scene&& scene) noexcept;
    bool render(uint32_t imageIndex);

    void render();
    ImageData& renderTargetCpuData();

    VulkanRenderer& renderer;
    // std::vector<Camera> cameras;
    Camera camera;
    VkClearColorValue clearColor;
    std::vector<Drawable> drawables;
    // Lights as well eventually

    VkDescriptorPool descriptorPool;
};

#endif /* RENDERER_SCENE_HPP_ */
