#include "CameraManager.h"

#include "Core/Profiler.h"
#include "GameContext.h"
#include "Graphics/Renderer.h"

CameraManager* g_pCameraManager = nullptr;

CameraManager::CameraManager()
{
	g_pCameraManager = this;
}

CameraManager::~CameraManager()
{
	g_pCameraManager = nullptr;
}

void CameraManager::Update( const GameContext& oGameContext )
{
	ProfilerBlock oBlock( "CameraManager" );

	if( oGameContext.m_bEditing == false && m_xActiveCamera.IsValid() )
		m_xActiveCamera->ApplyCamera( g_pRenderer->m_oCamera );
	else
		m_oFreeCamera.Update( oGameContext.m_fLastRealDeltaTime );
}
