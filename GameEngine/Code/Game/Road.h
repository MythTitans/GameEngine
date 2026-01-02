#pragma once

#include "Game/Component.h"
#include "Game/ResourceTypes.h"
#include "Graphics/Mesh.h"

class RenderContext;
class SplineComponent;
struct RoadNode;

class RoadComponent : public Component
{
public:
	friend class RoadTrenchComponent;

	explicit RoadComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Start() override;
	void Update( const GameContext& oGameContext ) override;
	void Stop() override;
	void Dispose() override;

	void DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	bool DisplayInspector() override;

	void GenerateRoad();
	void GenerateRoad( const Spline& oLeftSpline, const Spline& oRightSpline );
#endif

private:

	using SplineHandle = ComponentHandle< SplineComponent >;
	SplineHandle		m_xSpline;

	TextureResPtr		m_xTexture;
	Mesh				m_oMesh;

	RoadNode* m_pRoadNode;

	float				m_fDistance = 1.f;
	float				m_fTolerance = 0.01f;
};
class RoadTrenchComponent : public Component
{
public:
	friend class TerrainComponent;

	explicit RoadTrenchComponent( Entity* pEntity );

	void			Initialize() override;
	void			Dispose() override;

	const RoadNode* GetRoadNode() const;

private:
	ComponentHandle< RoadComponent > m_xRoad;
};
