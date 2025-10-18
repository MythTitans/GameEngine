#pragma once

#include "FreeCamera.h"
#include "Camera.h"

struct GameContext;

class CameraManager
{
public:
	friend class Editor;

	CameraManager();
	~CameraManager();

	template < typename ComponentType >
	void SetActiveCamera( CameraComponentBase* pCamera )
	{
		m_xActiveCamera = pCamera;
		m_xActiveCamera.SetComponentSubType< ComponentType >();
	}

	void Update( const GameContext& oGameContext );

private:
	using CameraHandle = ComponentSubTypeHandle< CameraComponentBase >;
	CameraHandle	m_xActiveCamera;
	FreeCamera		m_oFreeCamera;
};

extern CameraManager* g_pCameraManager;