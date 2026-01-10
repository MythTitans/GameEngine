#pragma once

#include "Game/ResourceTypes.h"
#include "Mesh.h"
#include "Technique.h"

class RenderContext;
struct SkyNode;

class Skybox
{
public:
	Skybox();

	void Render( const SkyNode* pSky, const RenderContext& oRenderContext );

	bool OnLoading();
	void OnLoaded();

private:
	enum class SkyboxParam
	{
		VIEW_PROJECTION,
		CUBE_MAP,
		_COUNT
	};

	TechniqueResPtr				m_xSkybox;
	PARAM_SHEET( SkyboxParam )	m_oSkyboxSheet;

	Mesh						m_oMesh;
};