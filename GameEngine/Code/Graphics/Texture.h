#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

enum class TextureFormat
{
	RED,
	RGB,
	RGBA,
	NORMAL,
	DEPTH,
	ID
};

class Texture
{
public:
	friend class Renderer;
	friend class TextRenderer;
	friend class RenderTarget;

	Texture();

	void			Create( const int iWidth, const int iHeight, const TextureFormat eTextureFormat, const uint8* pData );
	void			Destroy();

	int				GetWidth() const;
	int				GetHeight() const;
	TextureFormat	GetFormat() const;

private:
	GLuint			m_uTextureID;
	int				m_iWidth;
	int				m_iHeight;
	TextureFormat	m_eFormat;
};