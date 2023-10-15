/*
 * Scene.hpp
 *
 *  Created on: Sep 14, 2023
 *      Author: nic
 */

#ifndef RENDERER_SCENE_HPP_
#define RENDERER_SCENE_HPP_

#include <vector>

class Camera;
class Drawable;
class VulkanRenderer;

class Scene
{
public:
	explicit Scene(VulkanRenderer& renderer);
	bool render(uint32_t imageIndex);

	VulkanRenderer& renderer;
	std::vector<Camera> cameras;
	std::vector<Drawable> drawables;
	// Lights as well eventually
};


#endif /* RENDERER_SCENE_HPP_ */
