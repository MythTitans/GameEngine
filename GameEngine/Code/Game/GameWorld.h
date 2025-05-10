#pragma once

#include "Core/Types.h"
#include "FreeCamera.h"
#include "Scene.h"

struct GameContext;

class GameWorld
{
public:
	friend class Editor;
	friend class Entity;

	GameWorld();
	~GameWorld();

	void Load( const std::string& sFilePath );
	void Run();
	void Reset();

	void Update( const GameContext& oGameContext );

	bool IsReady() const;

private:
	void UpdateWorld( const GameContext& oGameContext );

	enum class WorldState : uint8
	{
		EMPTY,
		LOADING,
		READY,
		RUNNING
	};

	enum class WorldTrigger : uint8
	{
		NONE,
		LOAD,
		RESET,
		RUN
	};

	WorldState		m_eWorldState;
	WorldTrigger	m_eWorldTrigger;

	std::string		m_sSceneFilePath;
	Scene			m_oScene;

	FreeCamera		m_oFreeCamera;
};

extern GameWorld* g_pGameWorld;
