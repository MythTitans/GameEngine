#pragma once

#include <nlohmann/json.hpp>

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

	void	Load( const nlohmann::json& oJsonContent );
	void	Run();
	void	Reset();

	void	Update( const GameContext& oGameContext );

	bool	IsReady() const;

	Entity*	CreateEntity( const std::string& sName, Entity* pParent = nullptr );
	void	RemoveEntity( Entity* pEntity );

private:
	void	UpdateWorld( const GameContext& oGameContext );

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

	Scene			m_oScene;
	nlohmann::json	m_oSceneJson;

	FreeCamera		m_oFreeCamera;
};

extern GameWorld* g_pGameWorld;
