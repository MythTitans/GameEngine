#include "Skinning.h"

#include "Core/Array.h"
#include "Core/Common.h"
#include "Math/GLMHelpers.h"

GPUSkinningStorage::GPUSkinningStorage()
	: m_uCurrentOffset( 0 )
{
}

uint GPUSkinningStorage::Store( const Array< glm::mat4x3 >& aMatrices )
{
	const uint uBaseIndex = m_uCurrentOffset;
	ASSERT( m_uCurrentOffset + aMatrices.Count() < MAX_BONE_COUNT );

	for( uint u = 0; u < aMatrices.Count(); ++u )
		m_oSkinningData.m_aBones[ u + m_uCurrentOffset ].m_mMatrix = ToMat4( aMatrices[ u ] );

	m_uCurrentOffset += aMatrices.Count();

	return uBaseIndex;
}

void GPUSkinningStorage::Reset()
{
	m_uCurrentOffset = 0;
}

const GPUSkinningDataBlock& GPUSkinningStorage::GetSkinningData() const
{
	return m_oSkinningData;
}
