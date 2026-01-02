#pragma once

#ifdef EDITOR

#include <GL/glew.h>

#include "Core/Array.h"
#include "Core/Types.h"

enum class GizmoType : uint8;
enum class GizmoAxis : uint8;
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

#endif
