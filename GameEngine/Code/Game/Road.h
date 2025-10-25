#pragma once

#include "Component.h"
#include "Graphics/Mesh.h"
#include "ResourceLoader.h"

class SplineComponent;

class RoadComponent : public Component
{
public:
	RoadComponent( Entity* pEntity );

	void Initialize() override;
	bool IsInitialized() const override;
	void Update( const GameContext& oGameContext ) override;

#ifdef EDITOR
	bool DisplayInspector() override;

	void GenerateRoad();
#endif

private:

	using SplineHandle = ComponentHandle< SplineComponent >;
	SplineHandle		m_xSpline;

	TechniqueResPtr		m_xTechnique;
	Mesh				m_oMesh;
	MaterialReference	m_oMaterial;

	float				m_fDistance = 1.f;
	float				m_fTolerance = 0.01f;
};
