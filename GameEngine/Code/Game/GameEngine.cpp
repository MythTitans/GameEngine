#include "GameEngine.h"

#include "Core/Logger.h"

GameContext::GameContext()
	: m_uFrameIndex( 0 )
	, m_fLastDeltaTime( 0.f )
{
}

GameEngine* g_pGameEngine = nullptr;

GameEngine::GameEngine( const InputContext& oInputContext, const RenderContext& oRenderContext )
	: m_oInputContext( oInputContext )
	, m_oRenderContext( oRenderContext )
	, m_eGameState( GameState::INITIALIZING )
{
	g_pGameEngine = this;
}

GameEngine::~GameEngine()
{
	g_pGameEngine = nullptr;
}

Scene& GameEngine::GetScene()
{
	return m_oScene;
}

const Scene& GameEngine::GetScene() const
{
	return m_oScene;
}

const GameContext& GameEngine::GetGameContext() const
{
	return m_oGameContext;
}

void GameEngine::NewFrame()
{
	const GameTimePoint oNow = std::chrono::high_resolution_clock::now();
	if( m_oGameContext.m_uFrameIndex != 0 )
	{
		const uint64 uMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oNow - m_oGameContext.m_oFrameStart ).count();
		m_oGameContext.m_fLastDeltaTime = uMicroSeconds / 1000000.f;
	}
	m_oGameContext.m_oFrameStart = oNow;

	ImGui::NewFrame();

	m_oProfiler.NewFrame();
	m_oProfiler.Display();

	m_oDebugDisplay.NewFrame();
}

void GameEngine::ProcessFrame()
{
	ProfilerBlock oBlock( "GameEngine" );

	m_oResourceLoader.HandleLoadedResources();

	Update();

	m_oResourceLoader.ProcessLoadCommands();

	Render();

	++( m_oGameContext.m_uFrameIndex );
}

void GameEngine::EndFrame()
{
	ImGui::Render();

	Logger::Flush();
}

void GameEngine::Update()
{
	ProfilerBlock oBlock( "Update" );

	if( m_eGameState != GameState::INITIALIZING )
	{
		m_oDebugDisplay.DisplayText( CurrentStateStr(), glm::vec4( 0.f, 1.f, 0.f, 1.f ) );

		m_oInputHandler.UpdateInputs( m_oInputContext );

		if( m_eGameState == GameState::RUNNING )
		{
			m_oFreeCamera.Update( m_oGameContext.m_fLastDeltaTime );
			m_oEditor.Update( m_oInputContext, m_oRenderContext );
			m_oComponentManager.UpdateComponents( m_oGameContext.m_fLastDeltaTime );
		}
		else if( m_oComponentManager.AreComponentsInitialized() )
		{
			m_eGameState = GameState::RUNNING;
			m_oComponentManager.StartComponents();
		}
	}
	else
	{
		if( m_oRenderer.OnLoading() )
		{
			m_eGameState = GameState::LOADING;
			m_oComponentManager.InitializeComponents();
		}
	}
}

void GameEngine::Render()
{
	ProfilerBlock oBlock( "Render" );

	if( m_eGameState != GameState::INITIALIZING )
	{
		m_oRenderer.DisplayDebug();
		m_oRenderer.Render( m_oRenderContext );
		m_oDebugDisplay.Display( m_oGameContext.m_fLastDeltaTime, m_oRenderContext );
		m_oEditor.Render( m_oRenderContext );
		m_oDebugDisplay.DisplayOverlay( m_oGameContext.m_fLastDeltaTime, m_oRenderContext );
	}
}

const char* GameEngine::CurrentStateStr() const
{
	switch( m_eGameState )
	{
	case GameEngine::GameState::INITIALIZING:
		return "SETUP";
	case GameEngine::GameState::LOADING:
		return "LOADING";
	case GameEngine::GameState::RUNNING:
		return "RUNNING";
	}

	return "";
}
