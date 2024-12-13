#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

class Technique;

class TechniqueDefinitionBase
{
public:
	TechniqueDefinitionBase();
	~TechniqueDefinitionBase();

	void			Create( const Technique& oTechnique );

	bool			IsValid() const;

private:
	virtual void	CreateDefinition( const Technique& oTechnique ) = 0;

	bool m_bValid;
};

class DeferredMapsDefinition : public TechniqueDefinitionBase
{
public:
	DeferredMapsDefinition();

	void SetViewProjection( const glm::mat4& mViewProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseTexture( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLuint	m_uViewProjectionUniform;
	GLuint	m_uDiffuseColorUniform;
	GLuint	m_uDiffuseTextureUniform;
};

class DeferredComposeDefinition : public TechniqueDefinitionBase
{
public:
	DeferredComposeDefinition();

	void SetColor( const int iTextureUnit );
	void SetNormal( const int iTextureUnit );
	void SetDepth( const int iTextureUnit );
	void SetInverseViewProjection( const glm::mat4& mInverseViewProjection );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLuint m_uColorUniform;
	GLuint m_uNormalUniform;
	GLuint m_uDepthUniform;
	GLuint m_uInverseViewProjectionUniform;
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

	GLuint m_uPositionUniform;
	GLuint m_uSizeUniform;
	GLuint m_vGlyphOffsetUniform;
	GLuint m_vGlyphSizeUniform;
	GLuint m_vGlyphColorUniform;
	GLuint m_uAtlasTextureUniform;
};