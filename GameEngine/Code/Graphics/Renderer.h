#pragma once

#include "Core/Common.h"
#include "Core/ResourceLoader.h"
#include "Graphics/TechniqueDefinition.h"

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

	float				ComputeAspectRatio() const;

	void				OnFrameBufferResized( const int iWidth, const int iHeight );

	const RenderRect&	GetRenderRect() const { return m_oRenderRect; }

private:
	RenderRect	m_oRenderRect;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render( const RenderContext& oRenderContext );

private:
	TechniqueResPtr				m_xRenderTechnique;
	BasicTechniqueDefinition	m_oBasicTechniqueDefinition;
};

extern Renderer* g_pRenderer;