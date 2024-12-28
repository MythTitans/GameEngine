#pragma once

class InputContext;
class RenderContext;

class Editor
{
public:
	Editor();

	void Display( const InputContext& oInputContext, const RenderContext& oRenderContext );

private:
	bool			m_bDisplayEditor;
};