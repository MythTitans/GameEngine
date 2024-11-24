#include "Texture.h"

#include "Core/Profiler.h"

Texture::Texture()
	: m_uTextureID( GL_INVALID_VALUE )
{
}

void Texture::Create( const int iWidth, const int iHeight, const TextureFormat eTextureFormat, const uint8* pData )
{
	glGenTextures( 1, &m_uTextureID );
	glBindTexture( GL_TEXTURE_2D, m_uTextureID );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLint iFormat = GL_RGBA;
	switch( eTextureFormat )
	{
	case TextureFormat::RGBA:
		iFormat = GL_RGBA;
		break;
	}

	glTexImage2D( GL_TEXTURE_2D, 0, iFormat, iWidth, iHeight, 0, iFormat, GL_UNSIGNED_BYTE, pData );

	glGenerateMipmap( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Texture::Destroy()
{
	glDeleteTextures( 1, &m_uTextureID );
}
