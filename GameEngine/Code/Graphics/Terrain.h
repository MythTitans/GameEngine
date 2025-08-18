#pragma once

#include "Game/Component.h"
#include "Game/ResourceLoader.h"
#include "Mesh.h"
#include "PxPhysicsAPI.h"

class RenderContext;
struct TerrainNode;

class Terrain
{
public:
	Terrain();

	void Render( const TerrainNode* pTerrain, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	TechniqueResPtr m_xTerrain;
};

class TerrainComponent : public Component
{
public:
	explicit TerrainComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Update( const GameContext& oGameContext ) override;

private:
	void GenerateTerrain();

	physx::PxRigidStatic*	m_pRigidStatic;

	TextureResPtr			m_xHeightMap;
	Mesh					m_oMesh;

	TerrainNode*			m_pTerrain;
};
