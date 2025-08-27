#include "GameWorld.h"

#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Entity.h"
#include "GameEngine.h"

GameWorld* g_pGameWorld = nullptr;

GameWorld::GameWorld()
	: m_eWorldState( WorldState::EMPTY )
	, m_eWorldTrigger( WorldTrigger::NONE )
{
	g_pGameWorld = this;
}

GameWorld::~GameWorld()
{
	g_pGameWorld = nullptr;
}

void GameWorld::Load( const nlohmann::json& oJsonContent )
{
	m_oSceneJson = oJsonContent;
	m_eWorldTrigger = WorldTrigger::LOAD;
}

void GameWorld::Run()
{
	m_eWorldTrigger = WorldTrigger::RUN;
}

void GameWorld::Reset()
{
	m_eWorldTrigger = WorldTrigger::RESET;
}

void GameWorld::Update( const GameContext& oGameContext )
{
	ProfilerBlock oBlock( "GameWorld" );

	switch( m_eWorldState )
	{
	case WorldState::EMPTY:
		if( m_eWorldTrigger == WorldTrigger::LOAD )
		{
			m_oScene.Load( m_oSceneJson );
			g_pComponentManager->InitializeComponents();
			m_eWorldState = WorldState::LOADING;
			m_eWorldTrigger = WorldTrigger::NONE;
		}
		break;
	case WorldState::LOADING:
		if( g_pComponentManager->AreComponentsInitialized() )
		{
			m_eWorldState = WorldState::READY;
			m_eWorldTrigger = WorldTrigger::NONE;
		}
		break;
	case WorldState::READY:
		if( m_eWorldTrigger == WorldTrigger::RUN )
		{
			g_pResourceLoader->m_bDisableUnusedResourcesDestruction = false;
			g_pComponentManager->StartComponents();
			m_eWorldState = WorldState::RUNNING;
			m_eWorldTrigger = WorldTrigger::NONE;
		}
		break;
	case WorldState::RUNNING:
		if( m_eWorldTrigger == WorldTrigger::RESET )
		{
			g_pResourceLoader->m_bDisableUnusedResourcesDestruction = true;
			m_oScene.Clear(); 
			m_eWorldState = WorldState::EMPTY;
			m_eWorldTrigger = WorldTrigger::LOAD;
		}
		else
		{
			UpdateWorld( oGameContext );
		}
		break;
	}
}

bool GameWorld::IsReady() const
{
	return m_eWorldState == WorldState::READY;
}

Entity* GameWorld::CreateEntity( const std::string& sName, Entity* pParent /*= nullptr */ )
{
	Entity* pEntity = m_oScene.CreateEntity( sName );
	if( pParent != nullptr )
		m_oScene.AttachToParent( pEntity, pParent );

	return pEntity;
}

void GameWorld::RemoveEntity( Entity* pEntity )
{
	m_oScene.RemoveEntity( pEntity );
}

void GameWorld::UpdateWorld( const GameContext& oGameContext )
{
	g_pComponentManager->StartPendingComponents();

	for( uint u = 0; u < oGameContext.m_uLastTicks; ++u )
	{
		ProfilerBlock oBlock( "Tick" );
		g_pComponentManager->TickComponents();
		g_pComponentManager->NotifyBeforePhysicsOnComponents();
		g_pPhysics->Tick();
		g_pComponentManager->NotifyAfterPhysicsOnComponents();
	}

	{
		ProfilerBlock oBlock( "Camera" );
		m_oFreeCamera.Update( oGameContext.m_fLastRealDeltaTime );
	}

	{
		ProfilerBlock oBlock( "Update" );
		g_pComponentManager->UpdateComponents( oGameContext );
	}
}
