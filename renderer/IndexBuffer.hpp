/*
 * IndexBuffer.hpp
 *
 *  Created on: Sep 17, 2023
 *      Author: nic
 */

#ifndef RENDERER_INDEXBUFFER_HPP_
#define RENDERER_INDEXBUFFER_HPP_

#include "Buffer.hpp"

class IndexBuffer
{
  public:
    IndexBuffer(VulkanRenderer& renderer, const std::vector<uint16_t>& indexData);
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) = delete;
    ~IndexBuffer();

    size_t size;
    size_t count;
    Buffer buffer;

  private:
    VulkanRenderer& renderer;
};

#endif /* RENDERER_INDEXBUFFER_HPP_ */
