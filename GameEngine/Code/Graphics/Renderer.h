#pragma once

#include "Core/Common.h"

struct GLFWwindow;

struct RenderRect
{
	RenderRect();

	uint m_uX;
	uint m_uY;
	uint m_uWidth;
	uint m_uHeight;
};

class RenderContext
{
public:
	friend class Renderer;

	explicit RenderContext( GLFWwindow* pWindow );

	float				ComputeAspectRatio() const;

	void				OnFrameBufferResized( const int iWidth, const int iHeight );

	GLFWwindow*			GetWindow() const { return m_pWindow; }
	const RenderRect&	GetRenderRect() const { return m_oRenderRect; }

private:
	GLFWwindow* m_pWindow;

	RenderRect	m_oRenderRect;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render( const RenderContext& oRenderContext );
};

extern Renderer* g_pRenderer;