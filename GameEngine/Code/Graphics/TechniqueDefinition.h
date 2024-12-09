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

class PresentToScreenTechniqueDefinition : public TechniqueDefinitionBase
{
public:
	PresentToScreenTechniqueDefinition();

	void SetTexture( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLuint m_uTextureUniform;
};

class BasicTechniqueDefinition : public TechniqueDefinitionBase
{
public:
	BasicTechniqueDefinition();

	void SetView( const glm::mat4& mView );
	void SetProjection( const glm::mat4& mProjection );
	void SetDiffuseColor( const glm::vec3& vColor );
	void SetDiffuseTexture( const int iTextureUnit );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLuint	m_uViewUniform;
	GLuint	m_uProjectionUniform;
	GLuint	m_uDiffuseColorUniform;
	GLuint	m_uDiffuseTextureUniform;
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