#pragma once

#include <glm/glm.hpp>

#include "Core/Types.h"

class InputContext;
class RenderContext;

class Editor
{
public:
	Editor();
	~Editor();

	void Update( const InputContext& oInputContext, const RenderContext& oRenderContext );
	void Render( const RenderContext& oRenderContext );

private:
	uint64		m_uSelectedEntityID;
	uint64		m_uGizmoEntityID;

	glm::vec3	m_vDraggingStartWorldPosition;
	glm::vec2	m_vDraggingStartCursorPosition;

	bool		m_bDisplayEditor;
};

extern Editor* g_pEditor;