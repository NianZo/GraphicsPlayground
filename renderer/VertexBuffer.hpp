/*
 * VertexBuffer.hpp
 *
 *  Created on: Sep 16, 2023
 *      Author: nic
 */

#ifndef RENDERER_VERTEXBUFFER_HPP_
#define RENDERER_VERTEXBUFFER_HPP_

#include "Buffer.hpp"

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
	Buffer tempBuffer;
private:
	VulkanRenderer& renderer;
};

#endif /* RENDERER_VERTEXBUFFER_HPP_ */
