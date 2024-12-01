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

const Scene& GameEngine::GetScene() const
{
	return m_oScene;
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
}

void GameEngine::ProcessFrame()
{
	ProfilerBlock oBlock( "GameEngine" );

	{
		ProfilerBlock oBlock( "PreUpdate" );
		m_oResourceLoader.PreUpdate();
	}

	{
		ProfilerBlock oBlock( "Update" );
		m_oInputHandler.UpdateInputs( m_oInputContext );
		m_oFreeCamera.Update( m_oGameContext.m_fLastDeltaTime );
		//m_oResourceLoader.Update();
	}

	{
		ProfilerBlock oBlock( "PostUpdate" );
		m_oResourceLoader.PostUpdate();
	}

	{
		ProfilerBlock oBlock( "Render" );
		m_oRenderer.Render( m_oRenderContext );
	}

	++( m_oGameContext.m_uFrameIndex );
}

void GameEngine::EndFrame()
{
	ImGui::Render();

	Logger::Flush();
}
