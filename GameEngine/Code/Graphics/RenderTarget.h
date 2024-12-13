#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Texture.h"

class RenderTarget
{
public:
	friend class Renderer;

	RenderTarget();

	void			Create( const int iWidth, const int iHeight, const Array< TextureFormat >& aFormats, const bool bDepthMap );
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