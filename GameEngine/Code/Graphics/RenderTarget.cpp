#include "RenderTarget.h"

#include "Core/Logger.h"

RenderTarget::RenderTarget()
	: m_uFrameBufferID( GL_INVALID_VALUE )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
{
}

void RenderTarget::Create( const int iWidth, const int iHeight )
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_oColorTexture.Create( iWidth, iHeight, TextureFormat::RGB, nullptr );
	m_oDepthTexture.Create( iWidth, iHeight, TextureFormat::DEPTH, nullptr );

	glGenFramebuffers( 1, &m_uFrameBufferID );

	glBindFramebuffer( GL_FRAMEBUFFER, m_uFrameBufferID );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_oColorTexture.m_uTextureID, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_oDepthTexture.m_uTextureID, 0 );

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		LOG_ERROR( "Failed to initialize frame buffer" );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RenderTarget::Destroy()
{
	glDeleteFramebuffers( 1, &m_uFrameBufferID );

	m_oColorTexture.Destroy();
	m_oDepthTexture.Destroy();
}

int RenderTarget::GetWidth() const
{
	return m_iWidth;
}

int RenderTarget::GetHeight() const
{
	return m_iHeight;
}
