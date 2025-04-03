#include "Bloom.h"

#include "Renderer.h"

static const std::string PARAM_KERNEL( "kernel" );
static const std::string PARAM_DELTA( "delta" );
static const std::string PARAM_VERTICAL( "vertical" );
static const std::string PARAM_INPUT_TEXTURE( "inputTexture" );

Array< float > ComputeBlurKernel( const uint uRadius )
{
	Array< float > aKernel( 2 * uRadius + 1 );

	const float fSigma = uRadius / 3.f;
	float fKernelSum = 0.f;
	for( uint u = 0; u < aKernel.Count(); ++u )
	{
		const float fX = ( float )u - ( float )uRadius;
		const float fGaussian = ( float )( glm::exp( -glm::pow( fX, 2 ) / ( 2 * glm::pow( fSigma, 2 ) ) ) );

		aKernel[ u ] = fGaussian;
		fKernelSum += fGaussian;
	}

	for( float& fKernelValue : aKernel )
		fKernelValue /= fKernelSum;

	return aKernel;
}

Bloom::Bloom()
	: m_xBlur( g_pResourceLoader->LoadTechnique( "Shader/blur.tech" ) )
	, m_iIterations( 1 )
{
}

void Bloom::Render( const RenderTarget& oInput, const RenderTarget& oOutput, const RenderContext& oRenderContext )
{
	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();

	for( int i = 0; i < 2; ++i )
	{
		if( oRenderRect.m_uWidth != m_oBloomRT[ i ].GetWidth() || oRenderRect.m_uHeight != m_oBloomRT[ i ].GetHeight() )
		{
			m_oBloomRT[ i ].Destroy();
			m_oBloomRT[ i ].Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight, TextureFormat::RGB16 ) );
		}
	}

	g_pRenderer->CopyRenderTargetColor( oInput, 1, m_oBloomRT[ 0 ], 0 );

	Technique& oBlurTechnique = m_xBlur->GetTechnique();
	g_pRenderer->SetTechnique( oBlurTechnique );

	const Array< float > aKernel = ComputeBlurKernel( 32 );
	for( uint u = 0; u < aKernel.Count(); ++u )
		oBlurTechnique.SetArrayParameter( PARAM_KERNEL, aKernel[ u ], u );

	bool bVertical = false;

	for( int i = 0; i < 2 * m_iIterations; ++i )
	{
		if( bVertical )
		{
			g_pRenderer->SetRenderTarget( m_oBloomRT[ 0 ] );

			glClear( GL_COLOR_BUFFER_BIT );

			oBlurTechnique.SetParameter( PARAM_VERTICAL, true );
			oBlurTechnique.SetParameter( PARAM_DELTA, 1.f / oRenderContext.GetRenderRect().m_uHeight );

			g_pRenderer->SetTextureSlot( m_oBloomRT[ 1 ].GetColorMap( 0 ), 0 );
			oBlurTechnique.SetParameter( PARAM_INPUT_TEXTURE, 0 );
		}
		else
		{
			g_pRenderer->SetRenderTarget( m_oBloomRT[ 1 ] );

			glClear( GL_COLOR_BUFFER_BIT );

			oBlurTechnique.SetParameter( PARAM_VERTICAL, false );
			oBlurTechnique.SetParameter( PARAM_DELTA, 1.f / oRenderContext.GetRenderRect().m_uWidth );

			g_pRenderer->SetTextureSlot( m_oBloomRT[ 0 ].GetColorMap( 0 ), 0 );
			oBlurTechnique.SetParameter( PARAM_INPUT_TEXTURE, 0 );
		}

		g_pRenderer->RenderQuad();

		bVertical = !bVertical;
	}

	g_pRenderer->ClearTextureSlot( 0 );

	g_pRenderer->SetRenderTarget( oOutput );

	g_pRenderer->BlendTextures( oInput.GetColorMap( 0 ), m_oBloomRT[ 0 ].GetColorMap( 0 ) );
}

bool Bloom::OnLoading()
{
	return m_xBlur->IsLoaded();
}
