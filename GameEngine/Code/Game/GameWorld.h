#pragma once

#include <nlohmann/json.hpp>

#include "Core/Types.h"
#include "Scene.h"

struct GameContext;

class GameWorld
{
public:
	friend class Editor;
	friend class EntityHolder;

	GameWorld();
	~GameWorld();

	void							Load();
	void							Run();
	void							Reset();
									
	void							Update( const GameContext& oGameContext );
									
	bool							IsReady() const;
	bool							IsRunning() const;

	void							SetScene( const std::filesystem::path& oScenePath );
	const std::filesystem::path&	GetScene() const;
			
	Entity*							CreateEntity( const std::string& sName, Entity* pParent = nullptr );
	Entity*							CreateInternalEntity( const std::string& sName, Entity* pParent = nullptr );
	Entity*							FindEntity( const uint64 uEntityID );
	void							RemoveEntity( Entity* pEntity );

	void							AttachToParent( Entity* pChild, Entity* pParent );
	void							DetachFromParent( Entity* pChild );

private:
	void							UpdateWorld( const GameContext& oGameContext );

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

	WorldState				m_eWorldState;
	WorldTrigger			m_eWorldTrigger;

	std::filesystem::path	m_oScenePath;
	Scene					m_oScene;
	nlohmann::json			m_oSceneJson;
};

extern GameWorld* g_pGameWorld;
