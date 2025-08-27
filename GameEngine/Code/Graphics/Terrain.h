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
	void RenderWireframe( const TerrainNode* pTerrain, const RenderContext& oRenderContext, const glm::vec3 vColor );

	bool OnLoading();

private:
	TechniqueResPtr m_xTerrain;
};

class TerrainComponent : public Component
{
public:
	friend class TerrainChunkComponent;

	explicit TerrainComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Update( const GameContext& oGameContext ) override;
	void Stop() override;
	void Dispose() override;

	void DisplayGizmos( const bool bSelected ) override;
	void DisplayInspector() override;

private:
	void RegisterChunk( TerrainChunkComponent* pChunkComponent );
	void UnRegisterChunk( TerrainChunkComponent* pChunkComponent );

	PROPERTIES( TerrainComponent );
	PROPERTY_DEFAULT( "Width", m_fWidth, float, 100.f );
	PROPERTY_DEFAULT( "Height", m_fHeight, float, 100.f );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 10.f );
	PROPERTY_DEFAULT( "Width chunks", m_uWidthChunks, uint, 1 );
	PROPERTY_DEFAULT( "Height chunks", m_uHeightChunks, uint, 1 );

	Array< Entity* >		m_aTerrainChunks;

	TextureResPtr			m_xDiffuseMap;
	TextureResPtr			m_xHeightMap;

	TerrainNode*			m_pTerrain;
	physx::PxRigidStatic*	m_pRigidStatic;
};

class TerrainChunkComponent : public Component
{
public:
	friend class TerrainComponent;

	explicit TerrainChunkComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Stop() override;
	void Dispose() override;

	void DisplayGizmos( const bool bSelected ) override;
	void OnPropertyChanged( const std::string& sProperty ) override;

private:
	void UpdateChunk();
	void GenerateTerrain();
	void GenerateShape( const Array< glm::vec3 >& aVertices, const Array< uint >& aIndices );
	void DisplayWireMesh( const glm::vec3& vColor );

	PROPERTIES( TerrainChunkComponent );
	PROPERTY_DEFAULT( "Width resolution", m_uWidthResolution, uint, 100 );
	PROPERTY_DEFAULT( "Height resolution", m_uHeightResolution, uint, 100 );
	PROPERTY_DEFAULT( "Chunk index", m_iChunkIndex, int, -1 );

	ComponentHandle< TerrainComponent > m_hTerrainComponent;

	Mesh								m_oMesh;
	physx::PxShape*						m_pShape;
};
