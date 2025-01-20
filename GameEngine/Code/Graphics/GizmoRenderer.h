#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"
#include "Editor/Gizmo.h"
#include "TechniqueDefinition.h"

class RenderContext;

class GizmoRenderer
{
public:
	GizmoRenderer();
	~GizmoRenderer();

	void				RenderGizmo( const GizmoType eGizmoType, const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext );

private:
	void				RenderTranslationGizmo( const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext );
	void				RenderRotationGizmo( const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext );

	Array< GLfloat >	GenerateQuad( const GizmoAxis eGizmoAxis );
	Array< GLfloat >	GenerateArrow( const GizmoAxis eGizmoAxis );
	Array< GLfloat >	GenerateGiro( const GizmoAxis eGizmoAxis );

	GLuint m_uVertexArrayID;
	GLuint m_uVertexBufferID;
};
