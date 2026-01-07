#pragma comment( lib, "opengl32.lib" )

#define NOMINMAX
#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"

#include "Core/Logger.h"
#include "Game/GameEngine.h"
#include "Game/InputHandler.h"
#include "Graphics/Renderer.h"

static InputContext s_oInputContext;
static RenderContext s_oRenderContext;

void OnKeyEvent( GLFWwindow* /*pWindow*/, const int iKey, const int iScancode, const int iAction, const int iMods )
{
	s_oInputContext.OnKeyEvent( iKey, iScancode, iAction, iMods );
}

void OnCursorMoveEvent( GLFWwindow* /*pWindow*/, const double dCursorX, const double dCursorY )
{
	s_oInputContext.OnCursorMoveEvent( ( float )dCursorX, ( float )dCursorY );
}

void OnMouseButtonEvent( GLFWwindow* /*window*/, const int iButton, const int iAction, const int iMods )
{
	s_oInputContext.OnMouseEvent( iButton, iAction, iMods );
}

void OnWindowResizeEvent( GLFWwindow* /*pWindow*/, int iWidth, int iHeight )
{
	s_oRenderContext.OnFrameBufferResized( iWidth, iHeight );
}

int main()
{
	LOG_INFO( "Initializing GLFW " );
	if( glfwInit() == false )
	{
		LOG_ERROR( "Failed to initialize GLFW" );
		return -1;
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );

	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

	const int iWidth = 1920;
	const int iHeight = 1080;

	LOG_INFO( "Creating window" );
	GLFWwindow* pWindow = glfwCreateWindow( iWidth, iHeight, "Game engine", nullptr, nullptr );
	if( pWindow != nullptr )
	{
		int iFrameBufferWidth, iFrameBufferHeight;
		glfwGetFramebufferSize( pWindow, &iFrameBufferWidth, &iFrameBufferHeight );

		s_oRenderContext.OnFrameBufferResized( iFrameBufferWidth, iFrameBufferHeight );

		glfwMakeContextCurrent( pWindow );
		glfwSwapInterval( 0 );

		glewExperimental = GL_TRUE;

		LOG_INFO( "Initializing GLEW" );
		if( glewInit() != GLEW_OK )
			LOG_ERROR( "Failed to initialize GLEW" );

		glfwSetKeyCallback( pWindow, OnKeyEvent );
		glfwSetCursorPosCallback( pWindow, OnCursorMoveEvent );
		glfwSetMouseButtonCallback( pWindow, OnMouseButtonEvent );
		glfwSetWindowSizeCallback( pWindow, OnWindowResizeEvent );

		glfwSetInputMode( pWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE );

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		LOG_INFO( "Initializing ImGui" );
		if( ImGui_ImplGlfw_InitForOpenGL( pWindow, true ) == false || ImGui_ImplOpenGL3_Init( "#version 330" ) == false )
			LOG_ERROR( "Failed to initialize ImGui" );

		{
			GameEngine oGameEngine( s_oInputContext, s_oRenderContext );
			while( glfwWindowShouldClose( pWindow ) == false )
			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();

				oGameEngine.NewFrame();

				{
					ProfilerBlock oBlock( "Frame" );
					GPUProfilerBlock oGPUBlock( "Frame" );

					s_oInputContext.Refresh();
					oGameEngine.ProcessFrame();

					oGameEngine.EndFrame();

					{
						ProfilerBlock oBlock( "WaitDisplay" );
						GPUProfilerBlock oGPUBlock( "WaitDisplay" );

						{
							GPUMarker oGPUMarker( "ImGUI" );
							GPUProfilerBlock oGPUBlock( "ImGUI" );
							ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

							ImGui::UpdatePlatformWindows();
							ImGui::RenderPlatformWindowsDefault();
						}

						glfwMakeContextCurrent( pWindow );
					}

					glfwSwapBuffers( pWindow );
				}
			}
		}

		LOG_INFO( "Destroying ImGui" );
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		LOG_INFO( "Destroying window" );
		glfwDestroyWindow( pWindow );
	}
	else
	{
		LOG_ERROR( "Failed to create window" );
	}

	LOG_INFO( "Terminating GLFW" );
	glfwTerminate();

	Logger::Flush();
}