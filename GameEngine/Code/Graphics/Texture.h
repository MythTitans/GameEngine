#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

enum class TextureFormat
{
	RED,
	RGB,
	RGBA,
	RGB16,
	NORMAL,
	DEPTH,
	ID
};

struct TextureDesc
{
	TextureDesc( const int iWidth, const int iHeight, const TextureFormat eFormat );

	TextureDesc& Data( const uint8* pData );
	TextureDesc& Multisample( int8 iSamples );
	TextureDesc& SRGB( const bool bSRGB = true );
	TextureDesc& GenerateMips( const bool bGenerateMips = true );

	int				m_iWidth;
	int				m_iHeight;
	const uint8*	m_pData;
	TextureFormat	m_eFormat;
	int8			m_iSamples;
	bool			m_bSRGB;
	bool			m_bGenerateMips;
};

class Texture
{
public:
	friend class Renderer;
	friend class TextRenderer;
	friend class RenderTarget;

	Texture();

	void			Create( const TextureDesc& oDesc );
	void			Destroy();

	GLuint			GetID() const;
	int				GetWidth() const;
	int				GetHeight() const;
	TextureFormat	GetFormat() const;

private:
	GLuint			m_uTextureID;
	int				m_iWidth;
	int				m_iHeight;
	TextureFormat	m_eFormat;
};