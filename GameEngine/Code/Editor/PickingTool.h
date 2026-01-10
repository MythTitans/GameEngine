#pragma once

#ifdef EDITOR

#include "Core/Types.h"
#include "Game/ResourceTypes.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/ShaderBuffer.h"
#include "Graphics/Skinning.h"
#include "Graphics/Technique.h"

class RenderContext;

class PickingTool
{
public:
	PickingTool();

	bool	OnLoading();
	void	OnLoaded();

	uint64	Pick( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY, const bool bAllowGizmos );

private:
	enum class PickingParam : uint8
	{
		USE_SKINNING,
		MODEL_VIEW_PROJECTION,
		COLOR_ID,
		_COUNT
	};

	RenderTarget							m_oPickingTarget;

	ShaderBuffer< GPUSkinningDataBlock >	m_oSkinningBuffer;

	TechniqueResPtr							m_xPicking;
	PARAM_SHEET( PickingParam )				m_oPickingSheet;
};

#endif