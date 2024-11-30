#pragma once

#include "Core/Common.h"
#include "Core/Profiler.h"
#include "Core/ResourceLoader.h"
#include "Game/InputHandler.h"
#include "Game/Scene.h"
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
	const Scene&		GetScene() const;

	void				NewFrame();
	void				ProcessFrame();
	void				EndFrame();

private:
	ResourceLoader			m_oResourceLoader;
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	Profiler				m_oProfiler;

	Scene					m_oScene;

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	GameContext				m_oGameContext;
};

extern GameEngine* g_pGameEngine;