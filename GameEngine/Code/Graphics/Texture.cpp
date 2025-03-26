#include "Texture.h"

#include "Core/Profiler.h"

Texture::Texture()
	: m_uTextureID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_eFormat( TextureFormat::RGBA )
{
}

void Texture::Create( const int iWidth, const int iHeight, const TextureFormat eTextureFormat, const uint8* pData, const bool bSRGB /*= false*/ )
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
	GLenum eType = GL_UNSIGNED_BYTE;
	switch( eTextureFormat )
	{
	case TextureFormat::RED:
		iFormat = GL_RED;
		iInternalFormat = GL_R8;
		break;
	case TextureFormat::RGB:
		iFormat = GL_RGB;
		iInternalFormat = bSRGB ? GL_SRGB8 : GL_RGB;
		break;
	case TextureFormat::RGBA:
		iFormat = GL_RGBA;
		iInternalFormat = bSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
		break;
	case TextureFormat::RGB16:
		iFormat = GL_RGB;
		iInternalFormat = GL_RGB16F;
		break;
	case TextureFormat::NORMAL:
		iFormat = GL_RGB;
		iInternalFormat = GL_RGB16F;
		break;
	case TextureFormat::DEPTH:
		iFormat = GL_DEPTH_COMPONENT;
		iInternalFormat = GL_DEPTH_COMPONENT24;
		eType = GL_FLOAT;
		break;
	case TextureFormat::ID:
		iFormat = GL_RGBA_INTEGER;
		iInternalFormat = GL_RGBA16UI;
		break;
	}

	glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, iWidth, iHeight, 0, iFormat, eType, pData );

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

GLuint Texture::GetID() const
{
	return m_uTextureID;
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
