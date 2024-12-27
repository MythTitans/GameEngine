#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Technique.h"

class TechniqueDefinitionBase
{
public:
	TechniqueDefinitionBase();
	~TechniqueDefinitionBase();

	void			Create( const Technique& oTechnique );

	bool			IsValid() const;

	void			SetParameterID( const GLint iParameterID, const int iValue );
	void			SetParameterID( const GLint iParameterID, const float fValue );
	void			SetParameterID( const GLint iParameterID, const glm::vec2& vValue );
	void			SetParameterID( const GLint iParameterID, const glm::vec3& vValue );
	void			SetParameterID( const GLint iParameterID, const glm::vec4& vValue );
	void			SetParameterID( const GLint iParameterID, const glm::mat3& mValue );
	void			SetParameterID( const GLint iParameterID, const glm::mat4& mValue );

private:
	virtual void	CreateDefinition( const Technique& oTechnique ) = 0;

	bool m_bValid;
};

template < int iMaxDirectionalLightCount, int iMaxPointLightCount, int iMaxSpotLightCount >
class LightingDefinition : public TechniqueDefinitionBase
{
public:
	LightingDefinition()
		: m_iDirectionalLightCountUniform( -1 )
		, m_iPointLightCountUniform( -1 )
		, m_iSpotLightCountUniform( -1 )
	{
	}

	void SetDirectionalLights( const Array< glm::vec3 >& aLightDirections, const Array< glm::vec3 >& aLightColors, const Array< float >& aLightIntensities )
	{
		ASSERT( aLightDirections.Count() == aLightColors.Count() );
		ASSERT( aLightDirections.Count() == aLightIntensities.Count() );

		for( uint u = 0; u < aLightDirections.Count(); ++u )
		{
			SetParameterID( m_aDirectionalLightDirectionUniforms[ u ], aLightDirections[ u ] );
			SetParameterID( m_aDirectionalLightColorUniforms[ u ], aLightColors[ u ] );
			SetParameterID( m_aDirectionalLightIntensityUniforms[ u ], aLightIntensities[ u ] );
		}

		SetParameterID( m_iDirectionalLightCountUniform, ( int )aLightDirections.Count() );
	}

	void SetPointLights( const Array< glm::vec3 >& aLightPositions, const Array< glm::vec3 >& aLightColors, const Array< float >& aLightIntensities, const Array< float >& aLightFalloffFactors )
	{
		ASSERT( aLightPositions.Count() == aLightColors.Count() );
		ASSERT( aLightPositions.Count() == aLightIntensities.Count() );
		ASSERT( aLightPositions.Count() == aLightFalloffFactors.Count() );

		for( uint u = 0; u < aLightPositions.Count(); ++u )
		{
			SetParameterID( m_aPointLightPositionUniforms[ u ], aLightPositions[ u ] );
			SetParameterID( m_aPointLightColorUniforms[ u ], aLightColors[ u ] );
			SetParameterID( m_aPointLightIntensityUniforms[ u ], aLightIntensities[ u ] );
			SetParameterID( m_aPointLightFalloffFactorUniforms[ u ], aLightFalloffFactors[ u ] );
		}

		SetParameterID( m_iPointLightCountUniform, ( int )aLightPositions.Count() );
	}

	void SetSpotLights( const Array< glm::vec3 >& aLightPositions, const Array< glm::vec3 >& aLightDirections, const Array< glm::vec3 >& aLightColors, const Array< float >& aLightIntensities, const Array< float >& aLightInnerAngles, const Array< float >& aLightOuterAngles, const Array< float >& aLightFalloffFactors )
	{
		ASSERT( aLightPositions.Count() == aLightColors.Count() );
		ASSERT( aLightPositions.Count() == aLightDirections.Count() );
		ASSERT( aLightPositions.Count() == aLightIntensities.Count() );
		ASSERT( aLightPositions.Count() == aLightInnerAngles.Count() );
		ASSERT( aLightPositions.Count() == aLightOuterAngles.Count() );
		ASSERT( aLightPositions.Count() == aLightFalloffFactors.Count() );

		for( uint u = 0; u < aLightPositions.Count(); ++u )
		{
			SetParameterID( m_aSpotLightPositionUniforms[ u ], aLightPositions[ u ] );
			SetParameterID( m_aSpotLightDirectionUniforms[ u ], aLightDirections[ u ] );
			SetParameterID( m_aSpotLightColorUniforms[ u ], aLightColors[ u ] );
			SetParameterID( m_aSpotLightIntensityUniforms[ u ], aLightIntensities[ u ] );
			SetParameterID( m_aSpotLightOuterRangeUniforms[ u ], glm::cos( glm::radians( aLightOuterAngles[ u ] / 2.f ) ) );
			SetParameterID( m_aSpotLightRangeUniforms[ u ], glm::cos( glm::radians( aLightInnerAngles[ u ] / 2.f ) ) - glm::cos( glm::radians( aLightOuterAngles[ u ] / 2.f ) ) );
			SetParameterID( m_aSpotLightFalloffFactorUniforms[ u ], aLightFalloffFactors[ u ] );
		}

		SetParameterID( m_iSpotLightCountUniform, ( int )aLightPositions.Count() );
	}

protected:
	void CreateDefinition( const Technique& oTechnique ) override
	{
		m_aDirectionalLightDirectionUniforms = oTechnique.GetParameterIDArray( "directionalLightDirections", iMaxDirectionalLightCount );
		m_aDirectionalLightColorUniforms = oTechnique.GetParameterIDArray( "directionalLightColors", iMaxDirectionalLightCount );
		m_aDirectionalLightIntensityUniforms = oTechnique.GetParameterIDArray( "directionalLightIntensities", iMaxDirectionalLightCount );

		m_aPointLightPositionUniforms = oTechnique.GetParameterIDArray( "pointLightPositions", iMaxPointLightCount );
		m_aPointLightColorUniforms = oTechnique.GetParameterIDArray( "pointLightColors", iMaxPointLightCount );
		m_aPointLightIntensityUniforms = oTechnique.GetParameterIDArray( "pointLightIntensities", iMaxPointLightCount );
		m_aPointLightFalloffFactorUniforms = oTechnique.GetParameterIDArray( "pointLightFalloffFactor", iMaxPointLightCount );

		m_aSpotLightPositionUniforms = oTechnique.GetParameterIDArray( "spotLightPositions", iMaxSpotLightCount );
		m_aSpotLightDirectionUniforms = oTechnique.GetParameterIDArray( "spotLightDirections", iMaxSpotLightCount );
		m_aSpotLightColorUniforms = oTechnique.GetParameterIDArray( "spotLightColors", iMaxSpotLightCount );
		m_aSpotLightIntensityUniforms = oTechnique.GetParameterIDArray( "spotLightIntensities", iMaxSpotLightCount );
		m_aSpotLightOuterRangeUniforms = oTechnique.GetParameterIDArray( "spotLightOuterRanges", iMaxSpotLightCount );
		m_aSpotLightRangeUniforms = oTechnique.GetParameterIDArray( "spotLightRanges", iMaxSpotLightCount );
		m_aSpotLightFalloffFactorUniforms = oTechnique.GetParameterIDArray( "spotLightFalloffFactors", iMaxSpotLightCount );

		m_iDirectionalLightCountUniform = oTechnique.GetParameterID( "directionalLightCount" );
		m_iPointLightCountUniform = oTechnique.GetParameterID( "pointLightCount" );
		m_iSpotLightCountUniform = oTechnique.GetParameterID( "spotLightCount" );
	}

	Array< GLint >	m_aDirectionalLightDirectionUniforms;
	Array< GLint >	m_aDirectionalLightColorUniforms;
	Array< GLint >	m_aDirectionalLightIntensityUniforms;

	Array< GLint >	m_aPointLightPositionUniforms;
	Array< GLint >	m_aPointLightColorUniforms;
	Array< GLint >	m_aPointLightIntensityUniforms;
	Array< GLint >	m_aPointLightFalloffFactorUniforms;

	Array< GLint >	m_aSpotLightPositionUniforms;
	Array< GLint >	m_aSpotLightDirectionUniforms;
	Array< GLint >	m_aSpotLightColorUniforms;
	Array< GLint >	m_aSpotLightIntensityUniforms;
	Array< GLint >	m_aSpotLightOuterRangeUniforms;
	Array< GLint >	m_aSpotLightRangeUniforms;
	Array< GLint >	m_aSpotLightFalloffFactorUniforms;

	GLint			m_iDirectionalLightCountUniform;
	GLint			m_iPointLightCountUniform;
	GLint			m_iSpotLightCountUniform;
};

class ForwardOpaqueDefinition : public LightingDefinition< 2, 8, 6 >
{
public:
	ForwardOpaqueDefinition();

	void SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseMap( const int iTextureUnit );
	void SetNormalMap( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint m_iModelViewProjectionUniform;
	GLint m_iModelUniform;
	GLint m_iModelInverseTransposeUniform;
	GLint m_iDiffuseColorUniform;
	GLint m_iDiffuseMapUniform;
	GLint m_iNormalMapUniform;
};

class DeferredMapsDefinition : public TechniqueDefinitionBase
{
public:
	DeferredMapsDefinition();

	void SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseMap( const int iTextureUnit );
	void SetNormalMap( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint m_iModelViewProjectionUniform;
	GLint m_iModelInverseTransposeUniform;
	GLint m_iDiffuseColorUniform;
	GLint m_iDiffuseMapUniform;
	GLint m_iNormalMapUniform;
};

// TODO #eric use SSBO to store light data
class DeferredComposeDefinition : public LightingDefinition< 2, 8, 6 >
{
public:
	DeferredComposeDefinition();

	void SetColor( const int iTextureUnit );
	void SetNormal( const int iTextureUnit );
	void SetDepth( const int iTextureUnit );
	void SetInverseViewProjection( const glm::mat4& mInverseViewProjection );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint			m_iColorUniform;
	GLint			m_iNormalUniform;
	GLint			m_iDepthUniform;
	GLint			m_iInverseViewProjectionUniform;
};

class TextTechniqueDefinition : public TechniqueDefinitionBase
{
public:
	TextTechniqueDefinition();

	void SetScreenPosition( const glm::vec2& vPosition );
	void SetScreenSize( const glm::vec2& vSize );
	void SetGlyph( const glm::vec2& vOffset, const glm::vec2& vSize );
	void SetColor( const glm::vec4& vColor );
	void SetTexture( const GLint iTexture );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint m_iPositionUniform;
	GLint m_iSizeUniform;
	GLint m_iGlyphOffsetUniform;
	GLint m_iGlyphSizeUniform;
	GLint m_iGlyphColorUniform;
	GLint m_iAtlasTextureUniform;
};