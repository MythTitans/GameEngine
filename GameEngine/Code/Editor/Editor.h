#pragma once

#include "Core/Types.h"

class InputContext;
class RenderContext;

class Editor
{
public:
	Editor();

	void Update( const InputContext& oInputContext, const RenderContext& oRenderContext );
	void Render( const RenderContext& oRenderContext );

private:
	uint64	m_uSelectedEntityID;

	bool	m_bDisplayEditor;
};