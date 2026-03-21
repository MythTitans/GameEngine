#pragma once

#ifdef EDITOR

#include "Graphics/RenderTarget.h"
#include "Resource/ResourceTypes.h"

class TerrainComponent;

class TrenchTool
{
public:
	TrenchTool();

	bool OnLoading();
	void OnLoaded();

	void Trench( const TerrainComponent* pTerrain, Texture& oTrench );

private:
	TechniqueResPtr m_xTrench;

	RenderTarget	m_oTrenchRT;
};

#endif