#pragma once

#include "Core/Common.h"
#include "Core/Profiler.h"
#include "Core/ResourceLoader.h"
#include "Graphics/Renderer.h"

class Renderer;

class GameEngine
{
public:
	explicit GameEngine( const RenderContext& oRenderContext );
	~GameEngine();

	void NewFrame();
	void ProcessFrame();
	void EndFrame();

private:
	Renderer				m_oRenderer;
	ResourceLoader			m_oResourceLoader;
	Profiler				m_oProfiler;

	const RenderContext&	m_oRenderContext;

	uint64					m_uFrameIndex;
};

extern GameEngine* g_pGameEngine;