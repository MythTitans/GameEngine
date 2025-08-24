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
	void Stop() override;
	void Dispose() override;

	void OnPropertyChanged( const std::string& sProperty ) override;

private:
	void GenerateTerrain();
	void GenerateShape( const Array< glm::vec3 >& aVertices, const Array< uint >& aIndices );

	PROPERTIES( TerrainComponent );
	PROPERTY_DEFAULT( "Width", m_fWidth, float, 100.f );
	PROPERTY_DEFAULT( "Height", m_fHeight, float, 100.f );
	PROPERTY_DEFAULT( "Width resolution", m_uWidthResolution, uint, 100 );
	PROPERTY_DEFAULT( "Height resolution", m_uHeightResolution, uint, 100 );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 10.f );

	TextureResPtr			m_xDiffuseMap;
	TextureResPtr			m_xHeightMap;
	Mesh					m_oMesh;

	TerrainNode*			m_pTerrain;
	physx::PxRigidStatic*	m_pRigidStatic;
};
