#pragma once

#include <GL/glew.h>

#include <glm/fwd.hpp>

#include "Core/Array.h"

class Technique;

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

class ForwardOpaqueDefinition : public TechniqueDefinitionBase
{
public:
	ForwardOpaqueDefinition();

	void SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseTexture( const int iTextureUnit );
	void SetLights( const Array< glm::vec3 >& aLightPositions, const Array< glm::vec3 >& aLightColors, const Array< float >& aLightIntensities, const Array< float >& aLightFalloffFactors );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint			m_iModelViewProjectionUniform;
	GLint			m_iModelUniform;
	GLint			m_iModelInverseTransposeUniform;
	GLint			m_iDiffuseColorUniform;
	GLint			m_iDiffuseTextureUniform;
	Array< GLint >	m_aLightPositionUniforms;
	Array< GLint >	m_aLightColorUniforms;
	Array< GLint >	m_aLightIntensityUniforms;
	Array< GLint >	m_aLightFalloffFactorUniforms;
	GLint			m_iLightCountUniform;
};

class DeferredMapsDefinition : public TechniqueDefinitionBase
{
public:
	DeferredMapsDefinition();

	void SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseTexture( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint m_iModelViewProjectionUniform;
	GLint m_iModelInverseTransposeUniform;
	GLint m_iDiffuseColorUniform;
	GLint m_iDiffuseTextureUniform;
};

// TODO #eric use SSBO to store light data
class DeferredComposeDefinition : public TechniqueDefinitionBase
{
public:
	DeferredComposeDefinition();

	void SetColor( const int iTextureUnit );
	void SetNormal( const int iTextureUnit );
	void SetDepth( const int iTextureUnit );
	void SetInverseViewProjection( const glm::mat4& mInverseViewProjection );
	void SetLights( const Array< glm::vec3 >& aLightPositions, const Array< glm::vec3 >& aLightColors, const Array< float >& aLightIntensities, const Array< float >& aLightFalloffFactors );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLint			m_iColorUniform;
	GLint			m_iNormalUniform;
	GLint			m_iDepthUniform;
	GLint			m_iInverseViewProjectionUniform;
	Array< GLint >	m_aLightPositionUniforms;
	Array< GLint >	m_aLightColorUniforms;
	Array< GLint >	m_aLightIntensityUniforms;
	Array< GLint >	m_aLightFalloffFactorUniforms;
	GLint			m_iLightCountUniform;
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