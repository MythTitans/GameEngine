#pragma comment( lib, "opengl32.lib" )

#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Core/Logger.h"
#include "Game/GameEngine.h"
#include "Graphics/Renderer.h"
#include "ImGui/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"

static RenderContext* s_pRenderContext = nullptr;

void OnKeyEvent( GLFWwindow* /*pWindow*/, const int /*iKey*/, const int /*iScancode*/, const int /*iAction*/, const int /*iMods*/ )
{

}

void OnCursorMoveEvent( GLFWwindow* /*pWindow*/, const double /*iXPos*/, const double /*iYPos*/ )
{

}

void OnWindowResizeEvent( GLFWwindow* /*pWindow*/, int iWidth, int iHeight )
{
	s_pRenderContext->OnFrameBufferResized( iWidth, iHeight );
}

int main()
{
	LOG_INFO( "Initializing GLFW " );
	if( !glfwInit() )
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

		RenderContext oRenderContext( pWindow );
		s_pRenderContext = &oRenderContext;

		oRenderContext.OnFrameBufferResized( iFrameBufferWidth, iFrameBufferHeight );

		glfwMakeContextCurrent( pWindow );

		//glfwSetInputMode( pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		//glfwSetInputMode( pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
		glfwSetKeyCallback( pWindow, OnKeyEvent );
		glfwSetCursorPosCallback( pWindow, OnCursorMoveEvent );
		glfwSetWindowSizeCallback( pWindow, OnWindowResizeEvent );

		GameEngine oGameEngine( oRenderContext );
		while( !glfwWindowShouldClose( pWindow ) )
		{
			glfwPollEvents();

			oGameEngine.NewFrame();
			oGameEngine.ProcessFrame();
			oGameEngine.EndFrame();

			glfwSwapBuffers( pWindow );
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
}