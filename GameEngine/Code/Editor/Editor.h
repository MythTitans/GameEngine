#pragma once

#include <glm/glm.hpp>

#include "Core/Types.h"
#include "Math/MathUtils.h"

class InputContext;
class RenderContext;
class GizmoComponent;

class Editor
{
public:
	Editor();
	~Editor();

	void		Update( const InputContext& oInputContext, const RenderContext& oRenderContext );
	void		Render( const RenderContext& oRenderContext );

private:
	Ray			ComputeCursorViewRay( const InputContext& oInputContext, const RenderContext& oRenderContext ) const;
	glm::vec3	ProjectOnGizmo( const Ray& oRay, const GizmoComponent& oGizmo ) const;

	uint64		m_uSelectedEntityID;
	uint64		m_uGizmoEntityID;

	glm::vec3	m_vInitialEntityWorldPosition;
	glm::vec3	m_vMoveStartWorldPosition;

	bool		m_bDisplayEditor;
};

extern Editor* g_pEditor;