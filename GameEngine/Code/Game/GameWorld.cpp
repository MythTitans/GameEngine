#include "GameWorld.h"

#include "Core/FileUtils.h"
#include "Core/Profiler.h"
#include "Entity.h"
#include "GameContext.h"
#include "ResourceLoader.h"
#include "Physics/Physics.h"

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

void GameWorld::SetScene( const std::filesystem::path& oScenePath )
{
	m_oScenePath = oScenePath;
	m_oSceneJson = nlohmann::json::parse( ReadTextFile( m_oScenePath ) );
}

const std::filesystem::path& GameWorld::GetScene() const
{
	return m_oScenePath;
}

void GameWorld::Load()
{
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

bool GameWorld::IsRunning() const
{
	return m_eWorldState == WorldState::RUNNING;
}

Entity* GameWorld::CreateEntity( const std::string& sName, Entity* pParent /*= nullptr */ )
{
	Entity* pEntity = m_oScene.CreateEntity( sName );
	if( pParent != nullptr )
		m_oScene.AttachToParent( pEntity, pParent );

	return pEntity;
}

Entity* GameWorld::CreateInternalEntity( const std::string& sName, Entity* pParent /*= nullptr */ )
{
	Entity* pEntity = m_oScene.CreateInternalEntity( sName );
	if( pParent != nullptr )
		m_oScene.AttachToParent( pEntity, pParent );

	return pEntity;
}

Entity* GameWorld::FindEntity( const uint64 uEntityID )
{
	return m_oScene.FindEntity( uEntityID );
}

void GameWorld::RemoveEntity( Entity* pEntity )
{
	m_oScene.RemoveEntity( pEntity );
}

void GameWorld::AttachToParent( Entity* pChild, Entity* pParent )
{
	m_oScene.AttachToParent( pChild, pParent );
}

void GameWorld::DetachFromParent( Entity* pChild )
{
	m_oScene.DetachFromParent( pChild );
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
		ProfilerBlock oBlock( "Logic" );
		g_pComponentManager->UpdateComponents( oGameContext );
	}

	{
		ProfilerBlock oBlock( "Finalize" );
		g_pComponentManager->FinalizeComponents();
	}
}
