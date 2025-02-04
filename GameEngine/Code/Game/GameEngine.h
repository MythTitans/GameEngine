#pragma once

#include "ComponentManager.h"
#include "Core/Common.h"
#include "Core/MemoryTracker.h"
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

	Scene&					GetScene();
	const Scene&			GetScene() const;

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

private:
	Profiler				m_oProfiler;
	MemoryTracker			m_oMemoryTracker;

	ResourceLoader			m_oResourceLoader;
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	ComponentManager		m_oComponentManager;

	DebugDisplay			m_oDebugDisplay;
	Editor					m_oEditor;

	FreeCamera				m_oFreeCamera;

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	Scene					m_oScene;
	GameContext				m_oGameContext;
	GameState				m_eGameState;
};

extern GameEngine* g_pGameEngine;