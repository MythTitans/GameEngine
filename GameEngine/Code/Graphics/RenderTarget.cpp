#include "RenderTarget.h"

#include "Core/Logger.h"

RenderTarget::RenderTarget()
	: m_uFrameBufferID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_uColorMapCount( 0 )
	, m_bDepthMap( false )
{
}

void RenderTarget::Create( const int iWidth, const int iHeight, const Array< TextureFormat >& aFormats, const bool bDepthMap )
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_uColorMapCount = aFormats.Count();
	m_bDepthMap = bDepthMap;

	m_aTextures.Resize( aFormats.Count() );
	for( uint u = 0; u < aFormats.Count(); ++u )
		m_aTextures[ u ].Create( iWidth, iHeight, aFormats[ u ], nullptr);

	if( bDepthMap )
	{
		m_aTextures.PushBack();
		m_aTextures.Back().Create( iWidth, iHeight, TextureFormat::DEPTH, nullptr );
	}

	glGenFramebuffers( 1, &m_uFrameBufferID );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uFrameBufferID );

	for( uint u = 0; u < aFormats.Count(); ++u )
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + u, GL_TEXTURE_2D, m_aTextures[ u ].m_uTextureID, 0 );

	if( bDepthMap )
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_aTextures[ m_uColorMapCount ].m_uTextureID, 0 );

	Array< GLuint > aDrawBuffers( m_uColorMapCount );
	for( uint u = 0; u < aDrawBuffers.Count(); ++u )
		aDrawBuffers[ u ] = GL_COLOR_ATTACHMENT0 + u;

	glDrawBuffers( 2, aDrawBuffers.Data() );

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		LOG_ERROR( "Failed to initialize frame buffer" );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RenderTarget::Destroy()
{
	glDeleteFramebuffers( 1, &m_uFrameBufferID );

	for( Texture& oTexture : m_aTextures )
		oTexture.Destroy();
}

int RenderTarget::GetWidth() const
{
	return m_iWidth;
}

int RenderTarget::GetHeight() const
{
	return m_iHeight;
}

const Texture& RenderTarget::GetColorMap( const uint uIndex ) const
{
	ASSERT( uIndex < m_uColorMapCount );
	return m_aTextures[ uIndex ];
}

const Texture& RenderTarget::GetDepthMap() const
{
	ASSERT( m_bDepthMap );
	return m_aTextures[ m_uColorMapCount ];
}
