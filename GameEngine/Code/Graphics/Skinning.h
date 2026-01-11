#pragma once

#include <span>
#include <glm/glm.hpp>

#include "Core/Types.h"

inline constexpr uint MAX_BONE_COUNT = 1024;

template < typename T >
class Array;

struct alignas( 16 ) GPUMatrix
{
	glm::mat4 m_mMatrix;
};

struct GPUSkinningDataBlock
{
	GPUMatrix m_aBones[ MAX_BONE_COUNT ];
};

class GPUSkinningStorage
{
public:
	GPUSkinningStorage();

	uint						Store( const Array< glm::mat4x3 >& aMatrices );
	void						Reset();

	const GPUSkinningDataBlock& GetSkinningData() const;

private:
	GPUSkinningDataBlock	m_oSkinningData;

	uint					m_uCurrentOffset;
};
