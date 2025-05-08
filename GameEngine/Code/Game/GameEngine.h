#pragma once

#include "ComponentManager.h"
#include "Core/Common.h"
#include "Core/MemoryTracker.h"
#include "Core/Profiler.h"
#include "Editor/Editor.h"
#include "FreeCamera.h"
#include "GameWorld.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Renderer.h"
#include "InputHandler.h"
#include "Physics/Physics.h"
#include "ResourceLoader.h"
#include "Scene.h"

class Renderer;

struct GameContext
{
	GameContext();

	GameTimePoint	m_oFrameStart;
	uint64			m_uFrameIndex;

	float			m_fLastDeltaTime;
	float			m_fLastRealDeltaTime;

	uint			m_uLastTicks;

	bool			m_bEditing;
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
		RUNNING,
		EDITING
	};

private:
	MemoryTracker			m_oMemoryTracker;
	Profiler				m_oProfiler;

	ResourceLoader			m_oResourceLoader;
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	MaterialManager			m_oMaterialManager;
	ComponentManager		m_oComponentManager;
	Physics					m_oPhysics;
	GameWorld				m_oGameWorld;

	DebugDisplay			m_oDebugDisplay;
	Editor					m_oEditor;

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	GameContext				m_oGameContext;
	GameState				m_eGameState;
};

extern GameEngine* g_pGameEngine;