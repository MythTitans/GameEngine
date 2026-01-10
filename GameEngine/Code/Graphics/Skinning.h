#pragma once

#include <glm/glm.hpp>

struct alignas( 16 ) GPUMatrix
{
	glm::mat4 m_mMatrix;
};

struct GPUSkinningDataBlock
{
	GPUMatrix m_aBones[ 128 ];
};
