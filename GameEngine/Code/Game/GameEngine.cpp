#include "GameEngine.h"

#include "Core/Logger.h"

GameContext::GameContext()
	: m_uFrameIndex( 0 )
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
}
