#pragma once

#include "Game/ResourceTypes.h"
#include "Technique.h"
#include "RenderTarget.h"

class RenderContext;
class Texture;

class Bloom
{
public:
	friend class Renderer;

	Bloom();

	void Render( const RenderTarget& oInput, const RenderTarget& oOutput, const RenderContext& oRenderContext );

	bool OnLoading();
	void OnLoaded();

private:
	enum class BlurParam
	{
		KERNEL,
		VERTICAL,
		DELTA,
		INPUT_TEXTURE,
		_COUNT
	};

	RenderTarget				m_oBloomRT[ 2 ];

	TechniqueResPtr				m_xBlur;
	PARAM_SHEET( BlurParam )	m_oBlurSheet;

	int							m_iIterations;
};
