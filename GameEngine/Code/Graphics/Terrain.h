#pragma once

#include "Game/Component.h"
#include "Game/ResourceTypes.h"
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
	friend class TerrainChunkComponent;

	explicit TerrainComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Update( const GameContext& oGameContext ) override;
	void Stop() override;
	void Dispose() override;

	void DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	bool DisplayInspector() override;
#endif

private:
	void RegisterChunk( TerrainChunkComponent* pChunk );
	void UnRegisterChunk( TerrainChunkComponent* pChunk );

	PROPERTIES( TerrainComponent );
	PROPERTY_DEFAULT( "Width", m_fWidth, float, 100.f );
	PROPERTY_DEFAULT( "Height", m_fHeight, float, 100.f );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 10.f );
	HIDDEN_PROPERTY_DEFAULT( "Width chunks", m_uWidthChunks, uint, 1 );
	HIDDEN_PROPERTY_DEFAULT( "Height chunks", m_uHeightChunks, uint, 1 );

	using TerrainChunkHandle = ComponentHandle< TerrainChunkComponent >;
	Array< TerrainChunkHandle >	m_aTerrainChunks;

	TextureResPtr				m_xDiffuseMap;
	TextureResPtr				m_xHeightMap;

	TerrainNode*				m_pTerrainNode;
	physx::PxRigidStatic*		m_pRigidStatic;
};

class TerrainChunkComponent : public Component
{
public:
	friend class TerrainComponent;

	explicit TerrainChunkComponent( Entity* pEntity );

	void			Initialize() override;
	bool			IsInitialized() const override;
	void			Start() override;
	void			Stop() override;
	void			Dispose() override;

	void			DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	void			OnPropertyChanged( const std::string& sProperty ) override;
#endif

private:
	enum class Border
	{
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	void			UpdateChunk();

	void			GenerateTerrain();
	void			GenerateShape( const Array< glm::vec3 >& aVertices, const Array< uint >& aIndices );

	Array< float >	FetchBorder( const Border eBorder ) const;

	void			DisplayWireMesh( const glm::vec3& vColor ) const;

	PROPERTIES( TerrainChunkComponent );
	PROPERTY_DEFAULT( "Width resolution", m_uWidthResolution, uint, 100 );
	PROPERTY_DEFAULT( "Height resolution", m_uHeightResolution, uint, 100 );
	PROPERTY_DEFAULT( "Has collisions", m_bHasCollisions, bool, false );
	HIDDEN_PROPERTY_DEFAULT( "Chunk index", m_iChunkIndex, int, -1 );

	ComponentHandle< TerrainComponent > m_hTerrain;

	Mesh								m_oMesh;
	physx::PxShape*						m_pShape;
};
