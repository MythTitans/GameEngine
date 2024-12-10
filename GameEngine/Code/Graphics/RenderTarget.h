#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Texture.h"

class RenderTarget
{
public:
	friend class Renderer;

	RenderTarget();

	void			Create( const int iWidth, const int iHeight );
	void			Destroy();

	int				GetWidth() const;
	int				GetHeight() const;

private:
	Texture			m_oColorTexture;
	Texture			m_oDepthTexture;
	GLuint			m_uFrameBufferID;
	int				m_iWidth;
	int				m_iHeight;
};