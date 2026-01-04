#pragma once

#include "CameraManager.h"
#include "ComponentManager.h"
#include "Core/MemoryTracker.h"
#include "Core/Profiler.h"
#include "Core/Types.h"
#include "Editor/Editor.h"
#include "GameWorld.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Renderer.h"
#include "InputHandler.h"
#include "Physics/Physics.h"
#include "ResourceLoader.h"

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
	friend class Editor;

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

	MemoryTracker			m_oMemoryTracker;
	Profiler				m_oProfiler;

	ResourceLoader			m_oResourceLoader;
	InputHandler			m_oInputHandler;
	Renderer				m_oRenderer;
	MaterialManager			m_oMaterialManager;
	ComponentManager		m_oComponentManager;
	Physics					m_oPhysics;
	GameWorld				m_oGameWorld;
	CameraManager			m_oCameraManager;

	DebugDisplay			m_oDebugDisplay;
#ifdef EDITOR
	Editor					m_oEditor;
#endif

	const InputContext&		m_oInputContext;
	const RenderContext&	m_oRenderContext;

	GameContext				m_oGameContext;
	GameState				m_eGameState;
};

extern GameEngine* g_pGameEngine;