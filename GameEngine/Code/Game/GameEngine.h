#pragma once

#include "ComponentManager.h"
#include "Core/Common.h"
#include "Core/Profiler.h"
#include "DebugDisplay.h"
#include "Editor/Editor.h"
#include "FreeCamera.h"
#include "Graphics/Renderer.h"
#include "InputHandler.h"
#include "ResourceLoader.h"
#include "Scene.h"

class Renderer;

struct GameContext
{
	GameContext();

	GameTimePoint	m_oFrameStart;
	uint64			m_uFrameIndex;

	float			m_fLastDeltaTime;
};

class GameEngine
{
public:
	GameEngine( const InputContext& oInputContext, const RenderContext& oRenderContext );
	~GameEngine();

	const GameContext&		GetGameContext() const;

	void					NewFrame();
	void					ProcessFrame();
	void					EndFrame();

private:
	void					Update();
	void					Render();

	const char*				CurrentStateStr() const;

	enum class GameState
	{
		INITIALIZING,
		LOADING,
		RUNNING
	};

public:
	ResourceLoader			m_oResourceLoader;
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	Profiler				m_oProfiler;
	Editor					m_oEditor;

	ComponentManager		m_oComponentManager;
	Scene					m_oScene;
	DebugDisplay			m_oDebugDisplay;

private:
	FreeCamera				m_oFreeCamera;

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	GameContext				m_oGameContext;

	GameState				m_eGameState;
};

extern GameEngine* g_pGameEngine;