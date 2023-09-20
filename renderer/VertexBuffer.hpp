/*
 * VertexBuffer.hpp
 *
 *  Created on: Sep 16, 2023
 *      Author: nic
 */

#ifndef RENDERER_VERTEXBUFFER_HPP_
#define RENDERER_VERTEXBUFFER_HPP_

#include "Buffer.hpp"

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
};

class VertexBuffer
{
public:
	VertexBuffer(VulkanRenderer& renderer, const std::vector<Vertex>& vertexData);
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&&) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&&) = delete;
	~VertexBuffer();

	size_t size;
	Buffer buffer;
private:
	VulkanRenderer& renderer;
};

#endif /* RENDERER_VERTEXBUFFER_HPP_ */
