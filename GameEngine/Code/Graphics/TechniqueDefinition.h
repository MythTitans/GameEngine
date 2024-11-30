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

class BasicTechniqueDefinition : public TechniqueDefinitionBase
{
public:
	BasicTechniqueDefinition();

	void SetView( const glm::mat4& mView );
	void SetProjection( const glm::mat4& mProjection );

private:
	void CreateDefinition( const Technique& oTechnique ) override;

	GLuint	m_uViewUniformLocation;
	GLuint	m_uProjectionUniformLocation;
};