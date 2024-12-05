#include "Texture.h"

#include "Core/Profiler.h"

Texture::Texture()
	: m_uTextureID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_eFormat( TextureFormat::RGBA )
{
}

void Texture::Create( const int iWidth, const int iHeight, const TextureFormat eTextureFormat, const uint8* pData )
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_eFormat = eTextureFormat;

	glGenTextures( 1, &m_uTextureID );
	glBindTexture( GL_TEXTURE_2D, m_uTextureID );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLint iFormat = GL_RGBA;
	GLint iInternalFormat = GL_RGBA;
	switch( eTextureFormat )
	{
	case TextureFormat::RGBA:
		iFormat = GL_RGBA;
		iInternalFormat = GL_RGBA;
		break;
	case TextureFormat::RED:
		iFormat = GL_RED;
		iInternalFormat = GL_R8;
		break;
	}

	glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, iWidth, iHeight, 0, iFormat, GL_UNSIGNED_BYTE, pData );

	glGenerateMipmap( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Texture::Destroy()
{
	glDeleteTextures( 1, &m_uTextureID );

	m_uTextureID = GL_INVALID_VALUE;
	m_iWidth = 0;
	m_iHeight = 0;
	m_eFormat = TextureFormat::RGBA;
}

int Texture::GetWidth() const
{
	return m_iWidth;
}

int Texture::GetHeight() const
{
	return m_iHeight;
}

TextureFormat Texture::GetFormat() const
{
	return m_eFormat;
}
