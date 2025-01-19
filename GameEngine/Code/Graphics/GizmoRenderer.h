#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"
#include "TechniqueDefinition.h"

class RenderContext;

class GizmoRenderer
{
public:
	GizmoRenderer();
	~GizmoRenderer();

	void				RenderGizmo( const GizmoComponent::GizmoType eGizmoType, const GizmoComponent::GizmoAxis eGizmoAxis, const RenderContext& oRenderContext );

private:
	void				RenderTranslationGizmo( const GizmoComponent::GizmoAxis eGizmoAxis, const RenderContext& oRenderContext );

	Array< GLfloat >	GenerateQuad( const GizmoComponent::GizmoAxis eGizmoAxis );
	Array< GLfloat >	GenerateArrow( const GizmoComponent::GizmoAxis eGizmoAxis );

	GLuint m_uVertexArrayID;
	GLuint m_uVertexBufferID;
};
