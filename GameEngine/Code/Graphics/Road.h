#pragma once

#include "Game/ResourceTypes.h"

class RenderContext;
struct RoadNode;

class Road
{
public:
	Road();

	void Render( const Array<RoadNode*>& aRoads, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	TechniqueResPtr m_xRoad;
};
