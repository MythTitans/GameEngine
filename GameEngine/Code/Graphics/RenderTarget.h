#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Texture.h"

struct RenderTargetDesc
{
	RenderTargetDesc( const int iWidth, const int iHeight );
	RenderTargetDesc( const int iWidth, const int iHeight, const TextureFormat eColorFormat );

	RenderTargetDesc& Multisample( int8 iSamples );
	RenderTargetDesc& AddColor( const TextureFormat eFormat );
	RenderTargetDesc& Depth( const bool bDepth = true );

	int						m_iWidth;
	int						m_iHeight;
	Array< TextureFormat >	m_aColorFormats;
	int8					m_iSamples;
	bool					m_bDepth;
};

class RenderTarget
{
public:
	friend class Renderer;

	RenderTarget();

	void			Create( const RenderTargetDesc& oDesc );
	void			Destroy();

	int				GetWidth() const;
	int				GetHeight() const;

	const Texture&	GetColorMap( const uint uIndex ) const;
	const Texture&	GetDepthMap() const;

private:
	Array< Texture >	m_aTextures;
	GLuint				m_uFrameBufferID;
	int					m_iWidth;
	int					m_iHeight;
	uint8				m_uColorMapCount;
	bool				m_bDepthMap;
};