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
	RenderTargetDesc& Depth( const bool bDepth = true, const int8 iCount = 1 );
	RenderTargetDesc& Depth( const TextureFormat eFormat, const int8 iCount = 1 );

	int						m_iWidth;
	int						m_iHeight;
	Array< TextureFormat >	m_aColorFormats;
	int8					m_iSamples;
	TextureFormat			m_eDepthFormat;
	bool					m_bDepth;
	int8					m_iDepthCount;
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