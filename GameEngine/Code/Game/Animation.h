#pragma once

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

#include "Core/Array.h"

template < typename T >
inline T Interpolate( const T& oA, const T& oB, const float fRatio )
{
	return glm::lerp( oA, oB, fRatio );
}

template <>
inline glm::quat Interpolate( const glm::quat& qA, const glm::quat& qB, const float fRatio )
{
	return glm::slerp( qA, qB, fRatio );
}

template < typename T >
struct AnimationCurve
{
	T Evaluate( const float fTime ) const
	{
		ASSERT( m_aTimes.Empty() == false );
		ASSERT( m_aTimes.Count() == m_aValues.Count() );

		if( fTime <= m_aTimes.Front() )
			return m_aValues.Front();

		if( fTime >= m_aTimes.Back() )
			return m_aValues.Back();

		uint uStartIndex = 0;
		uint uEndIndex = m_aValues.Count() - 1;
		uint uMiddleIndex = ( uEndIndex - uStartIndex ) / 2;

		while( uEndIndex - uStartIndex > 1 )
		{
			if( fTime <= m_aTimes[ uMiddleIndex ] )
				uEndIndex = uMiddleIndex;
			else
				uStartIndex = uMiddleIndex;

			uMiddleIndex = uStartIndex + ( uEndIndex - uStartIndex ) / 2;
		}

		const float fDistance = m_aTimes[ uEndIndex ] - m_aTimes[ uStartIndex ];
		const float fRatio = ( fTime - m_aTimes[ uStartIndex ] ) / fDistance;

		return Interpolate( m_aValues[ uStartIndex ], m_aValues[ uEndIndex ], fRatio );
	}

	Array< float >	m_aTimes;
	Array< T >		m_aValues;
};

struct NodeAnimation
{
	NodeAnimation();

	AnimationCurve< glm::vec3 > m_oPositionCurve;
	AnimationCurve< glm::quat>	m_oRotationCurve;
	AnimationCurve< glm::vec3 > m_oScaleCurve;
	uint						m_uMatrixIndex;
};

struct Animation 
{
	Animation();

	std::string				m_sName;
	float					m_fDuration;
	Array< NodeAnimation >	m_aNodeAnimations;
};

struct Skeleton
{
	Skeleton();

	void Update( const glm::mat4x3& mParentMatrix, const ArrayView< glm::mat4x3 > aAnimationMatrices ) const;

	Array< Skeleton >	m_aChildren;
	uint				m_uMatrixIndex;
};
