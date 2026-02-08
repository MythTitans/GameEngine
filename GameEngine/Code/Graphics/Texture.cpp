#include "Texture.h"

#include "Core/Profiler.h"

static void GetFormatDetails( const TextureFormat eFormat, const bool bSRGB, GLint& iFormat, GLint& iInternalFormat, GLenum& eType )
{
	iFormat = GL_RGBA;
	iInternalFormat = GL_RGBA;
	eType = GL_UNSIGNED_BYTE;

	switch( eFormat )
	{
	case TextureFormat::R:
		iFormat = GL_RED;
		iInternalFormat = GL_R8;
		break;
	case TextureFormat::R16:
		iFormat = GL_RED;
		iInternalFormat = GL_R16;
		eType = GL_UNSIGNED_SHORT;
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
		iInternalFormat = GL_RGB16F; // TODO #eric check if type shouldn't be GL_FLOAT here
		break;
	case TextureFormat::NORMAL:
		iFormat = GL_RGB;
		iInternalFormat = GL_RGB16F; // TODO #eric check if type shouldn't be GL_FLOAT here
		break;
	case TextureFormat::DEPTH:
		iFormat = GL_DEPTH_COMPONENT;
		iInternalFormat = GL_DEPTH_COMPONENT24;
		eType = GL_FLOAT;
		break;
	case TextureFormat::SHADOW:
		iFormat = GL_DEPTH_COMPONENT;
		iInternalFormat = GL_DEPTH_COMPONENT24;
		eType = GL_FLOAT;
		break;
	case TextureFormat::ID8:
		iFormat = GL_RED_INTEGER;
		iInternalFormat = GL_R8UI;
		break;
	case TextureFormat::ID64:
		iFormat = GL_RGBA_INTEGER;
		iInternalFormat = GL_RGBA16UI;
		break;
	}
}

constexpr uint GetFormatBytes( const TextureFormat eFormat )
{
	switch( eFormat )
	{
	case TextureFormat::R:
		return 1 * sizeof( uint8 );
	case TextureFormat::R16:
		return 1 * sizeof( uint16 );
	case TextureFormat::RGB:
		return 3 * sizeof( uint8 );
	case TextureFormat::RGBA:
		return 4 * sizeof( uint8 );
	case TextureFormat::RGB16:
		return 3 * sizeof( uint16 );
	case TextureFormat::NORMAL:
		return 3 * sizeof( uint16 );
	case TextureFormat::DEPTH:
		return 1 * sizeof( GLfloat );
	case TextureFormat::SHADOW:
		return 1 * sizeof( GLfloat );
	case TextureFormat::ID64:
		return 4 * sizeof( uint16 );
	default:
		ASSERT( false );
		return 0;
	}
}

TextureDesc::TextureDesc( const int iWidth, const int iHeight, const TextureFormat eFormat )
	: m_iWidth( iWidth )
	, m_iHeight( iHeight )
	, m_pData( nullptr )
	, m_eFormat( eFormat )
	, m_eHorizontalWrapping( TextureWrapping::REPEAT )
	, m_eVerticalWrapping( TextureWrapping::REPEAT )
	, m_eMinFiltering( TextureFiltering::LINEAR )
	, m_eMagFiltering( TextureFiltering::LINEAR )
	, m_oBorderColor( Color::Black() )
	, m_iSamples( 1 )
	, m_iCount( 1 )
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

TextureDesc& TextureDesc::Array( int8 iCount )
{
	m_iCount = iCount;
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

TextureDesc& TextureDesc::Filtering( const TextureFiltering eFiltering )
{
	m_eMinFiltering = eFiltering;
	m_eMagFiltering = eFiltering;
	return *this;
}

TextureDesc& TextureDesc::MinFiltering( const TextureFiltering eFiltering )
{
	m_eMinFiltering = eFiltering;
	return *this;
}

TextureDesc& TextureDesc::MagFiltering( const TextureFiltering eFiltering )
{
	m_eMagFiltering = eFiltering;
	return *this;
}

TextureDesc& TextureDesc::BorderColor( const Color& oBorderColor )
{
	m_oBorderColor = oBorderColor;
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

	const GLenum eSimpleTextureKind = oDesc.m_iCount > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
	const GLenum eFullTextureKind = [&]() -> GLenum
	{
		if( oDesc.m_iSamples > 1 )
			return oDesc.m_iCount > 1 ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE;

		return oDesc.m_iCount > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
	}();

	glBindTexture( eFullTextureKind, m_uTextureID );

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
		case TextureWrapping::BORDER:
			return GL_CLAMP_TO_BORDER;
		}

		return GL_REPEAT;
	};

	auto GetFilteringMode = []( const TextureFiltering eTextureFiltering ) {
		switch( eTextureFiltering )
		{
		case TextureFiltering::NEAREST:
			return GL_NEAREST;
		case TextureFiltering::LINEAR:
			return GL_LINEAR;
		}

		return GL_LINEAR;
	};

	const GLfloat aBorderColor[ 4 ] = { oDesc.m_oBorderColor.m_vColor.r, oDesc.m_oBorderColor.m_vColor.g , oDesc.m_oBorderColor.m_vColor.b, 1.f };
	glTexParameterfv( eFullTextureKind, GL_TEXTURE_BORDER_COLOR, aBorderColor );
	glTexParameteri( eFullTextureKind, GL_TEXTURE_WRAP_S, GetWrappingMode( oDesc.m_eHorizontalWrapping ) );
	glTexParameteri( eFullTextureKind, GL_TEXTURE_WRAP_T, GetWrappingMode( oDesc.m_eVerticalWrapping ) );

	glTexParameteri( eFullTextureKind, GL_TEXTURE_MIN_FILTER, GetFilteringMode( oDesc.m_eMinFiltering ) );
	glTexParameteri( eFullTextureKind, GL_TEXTURE_MAG_FILTER, GetFilteringMode( oDesc.m_eMagFiltering ) );

	if( oDesc.m_eFormat == TextureFormat::SHADOW )
	{
		glTexParameteri( eFullTextureKind, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
		glTexParameteri( eFullTextureKind, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	}

	GLint iFormat;
	GLint iInternalFormat;
	GLenum eType;
	GetFormatDetails( m_eFormat, oDesc.m_bSRGB, iFormat, iInternalFormat, eType );

	if( oDesc.m_iSamples > 1 )
	{
		if( oDesc.m_iCount > 1 )
			glTexImage3DMultisample( eFullTextureKind, oDesc.m_iSamples, iInternalFormat, m_iWidth, m_iHeight, oDesc.m_iCount, GL_TRUE );
		else
			glTexImage2DMultisample( eFullTextureKind, oDesc.m_iSamples, iInternalFormat, m_iWidth, m_iHeight, GL_TRUE );
	}
	else
	{
		if( oDesc.m_iCount > 1 )
			glTexImage3D( eFullTextureKind, 0, iInternalFormat, m_iWidth, m_iHeight, oDesc.m_iCount, 0, iFormat, eType, oDesc.m_pData );
		else
			glTexImage2D( eFullTextureKind, 0, iInternalFormat, m_iWidth, m_iHeight, 0, iFormat, eType, oDesc.m_pData );
	}

	if( oDesc.m_bGenerateMips )
		glGenerateMipmap( eSimpleTextureKind );

	glBindTexture( eFullTextureKind, 0 );
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

void Texture::FetchData( Array< uint8 >& aData, const bool bSRGB /*= false */ ) const
{
	aData.Resize( m_iWidth * m_iHeight * GetFormatBytes( m_eFormat ) );
	glBindTexture( GL_TEXTURE_2D, m_uTextureID );

	GLint iFormat;
	GLint iInternalFormat;
	GLenum eType;
	GetFormatDetails( m_eFormat, bSRGB, iFormat, iInternalFormat, eType );

	glGetTexImage( GL_TEXTURE_2D, 0, iFormat, eType, aData.Data() );

	glBindTexture( GL_TEXTURE_2D, 0 );
}

CubeMapDesc::CubeMapDesc( const int iWidth, const int iHeight, const TextureFormat eFormat )
	: m_iWidth( iWidth )
	, m_iHeight( iHeight )
	, m_pData( nullptr )
	, m_eFormat( eFormat )
	, m_eHorizontalWrapping( TextureWrapping::REPEAT )
	, m_eVerticalWrapping( TextureWrapping::REPEAT )
	, m_eDepthWrapping( TextureWrapping::REPEAT )
	, m_bSRGB( false )
{
}

CubeMapDesc& CubeMapDesc::Data( const uint8* pData, const Side eSide )
{
	if( eSide == Side::_COUNT )
	{
		for( uint u = 0; u < Side::_COUNT; ++u )
			m_pData[ u ] = pData;
	}
	else
	{
		m_pData[ eSide ] = pData;
	}

	return *this;
}

CubeMapDesc& CubeMapDesc::Wrapping( const TextureWrapping eWrapping )
{
	m_eHorizontalWrapping = eWrapping;
	m_eVerticalWrapping = eWrapping;
	m_eDepthWrapping = eWrapping;
	return *this;
}

CubeMapDesc& CubeMapDesc::HorizontalWrapping( const TextureWrapping eWrapping )
{
	m_eHorizontalWrapping = eWrapping;
	return *this;
}

CubeMapDesc& CubeMapDesc::VerticalWrapping( const TextureWrapping eWrapping )
{
	m_eVerticalWrapping = eWrapping;
	return *this;
}

CubeMapDesc& CubeMapDesc::DepthWrapping( const TextureWrapping eWrapping )
{
	m_eDepthWrapping = eWrapping;
	return *this;
}

CubeMapDesc& CubeMapDesc::SRGB( const bool bSRGB /*= true*/ )
{
	m_bSRGB = bSRGB;
	return *this;
}

CubeMapDesc& CubeMapDesc::GenerateMips( const bool bGenerateMips /*= true */ )
{
	m_bGenerateMips = bGenerateMips;
	return *this;
}

CubeMap::CubeMap()
	: m_uTextureID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_eFormat( TextureFormat::RGBA )
{
}

void CubeMap::Create( const CubeMapDesc& oDesc )
{
	m_iWidth = oDesc.m_iWidth;
	m_iHeight = oDesc.m_iHeight;
	m_eFormat = oDesc.m_eFormat;

	glGenTextures( 1, &m_uTextureID );

	glBindTexture( GL_TEXTURE_CUBE_MAP, m_uTextureID );

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

	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GetWrappingMode( oDesc.m_eHorizontalWrapping ) );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GetWrappingMode( oDesc.m_eVerticalWrapping ) );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GetWrappingMode( oDesc.m_eDepthWrapping ) );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLint iFormat;
	GLint iInternalFormat;
	GLenum eType;
	GetFormatDetails( m_eFormat, oDesc.m_bSRGB, iFormat, iInternalFormat, eType );

	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + u, 0, iInternalFormat, m_iWidth, m_iHeight, 0, iFormat, eType, oDesc.m_pData[ u ] );

	if( oDesc.m_bGenerateMips )
		glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
}

void CubeMap::Destroy()
{
	glDeleteTextures( 1, &m_uTextureID );

	m_uTextureID = GL_INVALID_VALUE;
	m_iWidth = 0;
	m_iHeight = 0;
	m_eFormat = TextureFormat::RGBA;
}

GLuint CubeMap::GetID() const
{
	return m_uTextureID;
}

int CubeMap::GetWidth() const
{
	return m_iWidth;
}

int CubeMap::GetHeight() const
{
	return m_iHeight;
}

TextureFormat CubeMap::GetFormat() const
{
	return m_eFormat;
}
