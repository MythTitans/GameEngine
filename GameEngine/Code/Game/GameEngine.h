#pragma once

#include "Core/Common.h"
#include "Core/Profiler.h"
#include "Core/ResourceLoader.h"
#include "Game/InputHandler.h"
#include "Graphics/Renderer.h"

class Renderer;

struct GameContext
{
	GameContext();

	uint64 m_uFrameIndex;
};

class GameEngine
{
public:
	GameEngine( const InputContext& oInputContext, const RenderContext& oRenderContext );
	~GameEngine();

	const GameContext&	GetGameContext() const;

	void				NewFrame();
	void				ProcessFrame();
	void				EndFrame();

private:
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	ResourceLoader			m_oResourceLoader;
	Profiler				m_oProfiler;

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	GameContext				m_oGameContext;
};

extern GameEngine* g_pGameEngine;