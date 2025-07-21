#include "Texture.h"

#include "Core/Profiler.h"

TextureDesc::TextureDesc( const int iWidth, const int iHeight, const TextureFormat eFormat )
	: m_iWidth( iWidth )
	, m_iHeight( iHeight )
	, m_pData( nullptr )
	, m_eFormat( eFormat )
	, m_eHorizontalWrapping( TextureWrapping::REPEAT )
	, m_eVerticalWrapping( TextureWrapping::REPEAT )
	, m_iSamples( 1 )
	, m_bSRGB( false )
	, m_bGenerateMips( false )
{
}

TextureDesc& TextureDesc::Data( const uint8* pData )
{
	m_pData = pData;
	return *this;
}

TextureDesc& TextureDesc::Multisample( int8 iSamples )
{
	m_iSamples = iSamples;
	return *this;
}

TextureDesc& TextureDesc::Wrapping( const TextureWrapping eWrapping )
{
	m_eHorizontalWrapping = eWrapping;
	m_eVerticalWrapping = eWrapping;
	return *this;
}

TextureDesc& TextureDesc::HorizontalWrapping( const TextureWrapping eWrapping )
{
	m_eHorizontalWrapping = eWrapping;
	return *this;
}

TextureDesc& TextureDesc::VerticalWrapping( const TextureWrapping eWrapping )
{
	m_eVerticalWrapping = eWrapping;
	return *this;
}

TextureDesc& TextureDesc::SRGB( const bool bSRGB /*= true*/ )
{
	m_bSRGB = bSRGB;
	return *this;
}

TextureDesc& TextureDesc::GenerateMips( const bool bGenerateMips /*= true*/ )
{
	m_bGenerateMips = bGenerateMips;
	return *this;
}

Texture::Texture()
	: m_uTextureID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_eFormat( TextureFormat::RGBA )
{
}

void Texture::Create( const TextureDesc& oDesc )
{
	m_iWidth = oDesc.m_iWidth;
	m_iHeight = oDesc.m_iHeight;
	m_eFormat = oDesc.m_eFormat;

	glGenTextures( 1, &m_uTextureID );

	if( oDesc.m_iSamples > 1 )
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, m_uTextureID );
	else
		glBindTexture( GL_TEXTURE_2D, m_uTextureID );

	auto GetWrappingMode = []( const TextureWrapping eTextureWrapping ) {
		switch( eTextureWrapping )
		{
		case TextureWrapping::REPEAT:
			return GL_REPEAT;
		case TextureWrapping::REPEAT_MIRROR:
			return GL_MIRRORED_REPEAT;
		case TextureWrapping::CLAMP:
			return GL_CLAMP_TO_EDGE;
		case TextureWrapping::CLAMP_MIRROR:
			return GL_MIRROR_CLAMP_TO_EDGE;
		}

		return GL_REPEAT;
	};

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetWrappingMode( oDesc.m_eHorizontalWrapping ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetWrappingMode( oDesc.m_eVerticalWrapping ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLint iFormat = GL_RGBA;
	GLint iInternalFormat = GL_RGBA;
	GLenum eType = GL_UNSIGNED_BYTE;
	switch( m_eFormat )
	{
	case TextureFormat::RED:
		iFormat = GL_RED;
		iInternalFormat = GL_R8;
		break;
	case TextureFormat::RGB:
		iFormat = GL_RGB;
		iInternalFormat = oDesc.m_bSRGB ? GL_SRGB8 : GL_RGB;
		break;
	case TextureFormat::RGBA:
		iFormat = GL_RGBA;
		iInternalFormat = oDesc.m_bSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
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

	if( oDesc.m_iSamples > 1 )
		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, oDesc.m_iSamples, iInternalFormat, m_iWidth, m_iHeight, GL_TRUE );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, iInternalFormat, m_iWidth, m_iHeight, 0, iFormat, eType, oDesc.m_pData );

	if( oDesc.m_bGenerateMips )
		glGenerateMipmap( GL_TEXTURE_2D );

	if( oDesc.m_iSamples > 1 )
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );
	else
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
