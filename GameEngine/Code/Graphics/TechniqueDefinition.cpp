#include "TechniqueDefinition.h"

#include <glm/gtc/type_ptr.hpp>

#include "Technique.h"

TechniqueDefinitionBase::TechniqueDefinitionBase()
	: m_bValid( false )
{
}

TechniqueDefinitionBase::~TechniqueDefinitionBase()
{
	m_bValid = false;
}

void TechniqueDefinitionBase::Create( const Technique& oTechnique )
{
	CreateDefinition( oTechnique );

	m_bValid = true;
}

bool TechniqueDefinitionBase::IsValid() const
{
	return m_bValid;
}

BasicTechniqueDefinition::BasicTechniqueDefinition()
	: m_uViewUniformLocation( GL_INVALID_VALUE )
	, m_uProjectionUniformLocation( GL_INVALID_VALUE )
{
}

void BasicTechniqueDefinition::SetView( const glm::mat4& mView )
{
	glUniformMatrix4fv( m_uViewUniformLocation, 1, GL_FALSE, glm::value_ptr( mView ) );
}

void BasicTechniqueDefinition::SetProjection( const glm::mat4& mProjection )
{
	glUniformMatrix4fv( m_uProjectionUniformLocation, 1, GL_FALSE, glm::value_ptr( mProjection ) );
}

void BasicTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_uViewUniformLocation = oTechnique.GetUniformLocation( "view" );
	m_uProjectionUniformLocation = oTechnique.GetUniformLocation( "projection" );
}
