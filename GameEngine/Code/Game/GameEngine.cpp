#include "GameEngine.h"

#include "Core/Logger.h"

#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"

GameEngine* g_pGameEngine = nullptr;

GameEngine::GameEngine( const RenderContext& oRenderContext )
	: m_oRenderContext( oRenderContext )
	, m_uFrameIndex( 0 )
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	LOG_INFO( "Initializing ImGui" );
	if( ImGui_ImplGlfw_InitForOpenGL( oRenderContext.GetWindow(), true ) == false || ImGui_ImplOpenGL3_Init( "#version 330" ) == false )
		LOG_ERROR( "Failed to initialize ImGui" );

	g_pGameEngine = this;
}

GameEngine::~GameEngine()
{
	LOG_INFO( "Destroying ImGui" );
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	g_pGameEngine = nullptr;
}

void GameEngine::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_oProfiler.NewFrame();
}

void GameEngine::ProcessFrame()
{
	ProfilerBlock oBlock( "Frame" );

	{
		ProfilerBlock oBlock( "PreUpdate" );
		m_oResourceLoader.PreUpdate();
	}

	{
		ProfilerBlock oBlock( "Update" );
		m_oResourceLoader.Update();
	}

	{
		ProfilerBlock oBlock( "PostUpdate" );
		m_oResourceLoader.PostUpdate();
	}

	m_oRenderer.Render( m_oRenderContext );

	++m_uFrameIndex;
}

void GameEngine::EndFrame()
{
	m_oProfiler.Display();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}
