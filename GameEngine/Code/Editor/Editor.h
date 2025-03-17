#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

	void		ColorEdit( const char* sLabel, glm::vec3& vColor );

	uint64		m_uSelectedEntityID;
	uint64		m_uGizmoEntityID;

	glm::vec3	m_vInitialEntityPosition;
	glm::quat	m_qInitialEntityRotation;

	glm::vec3	m_vMoveStartPosition;
	glm::vec3	m_vRotationAxis;

	bool		m_bDisplayEditor;
};

extern Editor* g_pEditor;