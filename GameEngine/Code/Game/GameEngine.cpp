#include "GameEngine.h"

#include "Core/FileUtils.h"
#include "Core/Logger.h"

GameContext::GameContext()
	: m_uFrameIndex( 0 )
	, m_fLastDeltaTime( 0.f )
	, m_fLastRealDeltaTime( 0.f )
	, m_uLastTicks( 0 )
	, m_bEditing( false )
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
		m_oGameContext.m_fLastRealDeltaTime = glm::min( uMicroSeconds / 1000000.f, 1.f / 30.f );
	}
	m_oGameContext.m_oFrameStart = oNow;

	if( m_eGameState == GameState::EDITING )
	{
		m_oGameContext.m_fLastDeltaTime = 0.f;
		m_oGameContext.m_bEditing = true;
	}
	else
	{
		m_oGameContext.m_fLastDeltaTime = m_oGameContext.m_fLastRealDeltaTime;
		m_oGameContext.m_bEditing = false;
	}

	static float fAccumulatedTime = Physics::TICK_STEP;
	fAccumulatedTime += m_oGameContext.m_fLastDeltaTime;

	m_oGameContext.m_uLastTicks = 0;
	while( fAccumulatedTime >= Physics::TICK_STEP )
	{
		fAccumulatedTime -= Physics::TICK_STEP;
		++m_oGameContext.m_uLastTicks;
	}

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

	++m_oGameContext.m_uFrameIndex;
}

void GameEngine::EndFrame()
{
	m_oRenderer.Clear();

	glDisable( GL_FRAMEBUFFER_SRGB );
	ImGui::Render();

	Logger::Flush();
}

void GameEngine::Update()
{
	ProfilerBlock oBlock( "Update" );

	if( m_eGameState == GameState::INITIALIZING )
	{
		if( m_oRenderer.OnLoading() )
		{
			m_oRenderer.OnLoaded();

			const nlohmann::json oJsonContent = nlohmann::json::parse( ReadTextFile( std::filesystem::path( "Data/Scene/test.scene" ) ) );
			m_oGameWorld.Load( oJsonContent );
			m_eGameState = GameState::RUNNING;
		}
	}
	else
	{
		m_oDebugDisplay.DisplayText( CurrentStateStr(), glm::vec4( 0.f, 1.f, 0.f, 1.f ) );

		m_oInputHandler.UpdateInputs( m_oInputContext );

		if( m_oGameWorld.IsReady() )
		{
			m_oGameWorld.Run();
			m_eGameState = GameState::EDITING;

			m_oEditor.OnSceneLoaded();
		}

		m_oGameWorld.Update( m_oGameContext );

		m_oCameraManager.Update( m_oGameContext );
		
		m_oEditor.Update( m_oInputContext, m_oRenderContext );
	}
}

void GameEngine::Render()
{
	ProfilerBlock oBlock( "Render" );

	if( m_eGameState != GameState::INITIALIZING )
	{
		m_oRenderer.DisplayDebug();
		m_oResourceLoader.DisplayDebug();
		m_oRenderer.Render( m_oRenderContext );
		m_oDebugDisplay.Display( m_oRenderContext );
		m_oMemoryTracker.Display();
		m_oEditor.Render( m_oRenderContext );

		glDisable( GL_FRAMEBUFFER_SRGB );
		m_oDebugDisplay.DisplayOverlay( m_oGameContext.m_fLastDeltaTime, m_oRenderContext );
	}
}

const char* GameEngine::CurrentStateStr() const
{
	switch( m_eGameState )
	{
	case GameEngine::GameState::INITIALIZING:
		return "INITIALIZING";
	case GameEngine::GameState::RUNNING:
		return "RUNNING";
	case GameEngine::GameState::EDITING:
		return "EDITING";
	}

	return "";
}
