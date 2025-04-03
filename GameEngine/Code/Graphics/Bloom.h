#pragma once

#include "Game/ResourceLoader.h"
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

private:
	RenderTarget	m_oBloomRT[ 2 ];

	TechniqueResPtr	m_xBlur;

	int				m_iIterations;
};
