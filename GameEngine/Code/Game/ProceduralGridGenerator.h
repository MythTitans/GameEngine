#pragma once

#include <glm/glm.hpp>

#include "Game/Component.h"

class ProceduralGridGenerator : public Component
{
public:
	explicit ProceduralGridGenerator( Entity* pEntity );

	void Start() override;

#ifdef EDITOR
	bool DisplayInspector() override;
#endif

private:
	void Generate();
	void Clear();

	PROPERTIES( ProceduralGridGenerator );
	PROPERTY_DEFAULT( "Width", m_uWidth, uint, 10 );
	PROPERTY_DEFAULT( "Height", m_uHeight, uint, 10 );
	PROPERTY_DEFAULT( "Cell width", m_fCellWidth, float, 1.f );
	PROPERTY_DEFAULT( "Cell height", m_fCellHeight, float, 1.f );
	PROPERTY_DEFAULT( "Min variation X offset", m_vMinVariationXOffset, float, 0.f );
	PROPERTY_DEFAULT( "Max variation X offset", m_vMaxVariationXOffset, float, 0.f );
	PROPERTY_DEFAULT( "Min variation Y offset", m_vMinVariationYOffset, float, 0.f );
	PROPERTY_DEFAULT( "Max variation Y offset", m_vMaxVariationYOffset, float, 0.f );
	PROPERTY_DEFAULT( "Min variation Z offset", m_vMinVariationZOffset, float, 0.f );
	PROPERTY_DEFAULT( "Max variation Z offset", m_vMaxVariationZOffset, float, 0.f );
};