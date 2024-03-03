/*
 * UniformBuffer.hpp
 *
 *  Created on: Sep 20, 2023
 *      Author: nic
 */

#ifndef RENDERER_UNIFORMBUFFER_HPP_
#define RENDERER_UNIFORMBUFFER_HPP_

#include "Buffer.hpp"

class UniformBuffer
{
  public:
    UniformBuffer(VulkanRenderer& renderer, size_t size);
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&&) noexcept = default;
    UniformBuffer& operator=(UniformBuffer&&) = delete;
    ~UniformBuffer() = default;

    size_t size;
    Buffer buffer;
    void* data;

  private:
    VulkanRenderer& renderer;
};

#endif /* RENDERER_UNIFORMBUFFER_HPP_ */
