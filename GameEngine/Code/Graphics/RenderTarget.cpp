#include "RenderTarget.h"

#include "Core/Logger.h"

RenderTargetDesc::RenderTargetDesc( const int iWidth, const int iHeight )
	: m_iWidth( iWidth )
	, m_iHeight( iHeight )
	, m_iSamples( 1 )
	, m_bDepth( false )
{
}

RenderTargetDesc::RenderTargetDesc( const int iWidth, const int iHeight, const TextureFormat eColorFormat )
	: m_iWidth( iWidth )
	, m_iHeight( iHeight )
	, m_aColorFormats( 1, eColorFormat )
	, m_iSamples( 1 )
	, m_bDepth( false )
{
}

RenderTargetDesc& RenderTargetDesc::Multisample( int8 iSamples )
{
	m_iSamples = iSamples;
	return *this;
}

RenderTargetDesc& RenderTargetDesc::AddColor( const TextureFormat eFormat )
{
	m_aColorFormats.PushBack( eFormat );
	return *this;
}

RenderTargetDesc& RenderTargetDesc::Depth( const bool bDepth /*= true */ )
{
	m_bDepth = bDepth;
	return *this;
}

RenderTarget::RenderTarget()
	: m_uFrameBufferID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_uColorMapCount( 0 )
	, m_bDepthMap( false )
{
}

void RenderTarget::Create( const RenderTargetDesc& oDesc )
{
	m_iWidth = oDesc.m_iWidth;
	m_iHeight = oDesc.m_iHeight;
	m_uColorMapCount = oDesc.m_aColorFormats.Count();
	m_bDepthMap = oDesc.m_bDepth;

	m_aTextures.Resize( m_uColorMapCount );
	for( uint u = 0; u < m_uColorMapCount; ++u )
		m_aTextures[ u ].Create( TextureDesc( m_iWidth, m_iHeight, oDesc.m_aColorFormats[ u ] ).Multisample( oDesc.m_iSamples ).Wrapping( TextureWrapping::CLAMP ) );

	if( m_bDepthMap )
	{
		m_aTextures.PushBack();
		m_aTextures.Back().Create( TextureDesc( m_iWidth, m_iHeight, TextureFormat::DEPTH ).Multisample( oDesc.m_iSamples ).Wrapping( TextureWrapping::CLAMP ) );
	}

	glGenFramebuffers( 1, &m_uFrameBufferID );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uFrameBufferID );

	for( uint u = 0; u < oDesc.m_aColorFormats.Count(); ++u )
	{
		if( oDesc.m_iSamples > 1 )
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + u, GL_TEXTURE_2D_MULTISAMPLE, m_aTextures[ u ].GetID(), 0 );
		else
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + u, GL_TEXTURE_2D, m_aTextures[ u ].GetID(), 0 );
	}

	if( m_bDepthMap )
	{
		if( oDesc.m_iSamples > 1 )
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_aTextures[ m_uColorMapCount ].GetID(), 0 );
		else
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_aTextures[ m_uColorMapCount ].GetID(), 0 );
	}

	Array< GLuint > aDrawBuffers( m_uColorMapCount );
	for( uint u = 0; u < aDrawBuffers.Count(); ++u )
		aDrawBuffers[ u ] = GL_COLOR_ATTACHMENT0 + u;

	glDrawBuffers( m_uColorMapCount, aDrawBuffers.Data() );

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
