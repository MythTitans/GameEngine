#pragma once

#include "Game/ResourceTypes.h"

struct TerrainNode;

class RenderContext;

class Terrain
{
public:
	Terrain();

	void Render( const TerrainNode* pTerrain, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	TechniqueResPtr m_xTerrain;
};
