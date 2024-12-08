#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Core/Types.h"

enum class TextureFormat
{
	RED,
	RGB,
	RGBA,
};

class Texture
{
public:
	friend class Renderer;
	friend class TextRenderer;

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