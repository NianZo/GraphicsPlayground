/*
 * MeshData.hpp
 *
 *  Created on: Apr 16, 2022
 *      Author: nic
 */

#ifndef CHAPTER_3_HANDSHAKE_INCLUDE_MESHDATA_HPP_
#define CHAPTER_3_HANDSHAKE_INCLUDE_MESHDATA_HPP_

#include <array>

struct VertexWithColor
{
	float x, y, z, w; // Vertex position
	float r, g, b, a; // Color format Red, Green, Blue, Alpha
};

struct VertexWithUV
{
	float x, y, z, w;
	float u, v;
};

// Interleaved data containing position and color information
static const std::array<VertexWithColor, 3> triangleData =
{{
	{0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
	{1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
	{-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f}
}};

static const std::array<VertexWithColor, 4> squareData =
{{
	{-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
	{0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},
	{0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
	{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f}
}};

static const std::array<VertexWithColor, 36> cubeData =
{{
	{  1, -1, -1, 1.0f,		0.f, 0.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{  1,  1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{  1,  1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1,  1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },

	{  1, -1, 1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{  1,  1, 1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1, -1, 1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ -1, -1, 1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{  1,  1, 1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1,  1, 1, 1.0f,		1.f, 1.f, 1.f, 1.0f },

	{ 1, -1,  1, 1.0f,		1.f, 1.f, 1.f, 1.0f },
	{ 1, -1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ 1,  1,  1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ 1,  1,  1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{ 1, -1, -1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ 1,  1, -1, 1.0f,		1.f, 0.f, 0.f, 1.0f },

	{ -1, -1,  1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1,  1,  1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 1.f, 0.f, 1.0f },
	{ -1,  1,  1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1,  1, -1, 1.0f,		0.f, 0.f, 0.f, 1.0f },

	{  1, 1, -1, 1.0f,		1.f, 1.f, 1.f, 1.0f },
	{ -1, 1, -1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{  1, 1,  1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{  1, 1,  1, 1.0f,		1.f, 1.f, 0.f, 1.0f },
	{ -1, 1, -1, 1.0f,		0.f, 1.f, 1.f, 1.0f },
	{ -1, 1,  1, 1.0f,		0.f, 1.f, 0.f, 1.0f },

	{  1, -1, -1, 1.0f,		1.f, 0.f, 1.f, 1.0f },
	{  1, -1,  1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{ -1, -1, -1, 1.0f,		0.f, 0.f, 1.f, 1.0f },
	{  1, -1,  1, 1.0f,		1.f, 0.f, 0.f, 1.0f },
	{ -1, -1,  1, 1.0f,		0.f, 0.f, 0.f, 1.0f }
}};

static const std::array<VertexWithUV, 36> cubeUVData =
{{
	{ -1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f },  // -X side
	{ -1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f },
	{ -1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f },

	{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f },  // -Z side
	{  1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f },
	{  1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f },
	{ -1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 0.0f },
	{  1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 0.0f },

	{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f }, // -Y
	{  1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 0.0f },
	{  1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ -1.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ -1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f },

	{ -1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f },  // +Y side
	{ -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{  1.0f, 1.0f,-1.0f, 1.0f, 1.0f, 1.0f },

	{ 1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f },  // +X side
	{ 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ 1.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f },
	{ 1.0f, 1.0f,-1.0f, 1.0f, 0.0f, 1.0f },

	{ -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },  // +Z side
	{ -1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{  1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }
}};

static const std::array<uint16_t, 6> squareIndices = {{0, 3, 1, 3, 2, 1}};


#endif /* CHAPTER_3_HANDSHAKE_INCLUDE_MESHDATA_HPP_ */
