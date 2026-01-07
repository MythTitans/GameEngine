#include "Bloom.h"

#include "Core/Profiler.h"
#include "Game/ResourceLoader.h"
#include "Renderer.h"

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
	GPUMarker oGPUMarker( "Bloom" );
	GPUProfilerBlock oBlock( "Bloom" );

	RenderRect oBloomRenderRect = oRenderContext.GetRenderRect();
	oBloomRenderRect.m_uWidth /= 2;
	oBloomRenderRect.m_uHeight /= 2;

	glViewport( oBloomRenderRect.m_uX, oBloomRenderRect.m_uY, oBloomRenderRect.m_uWidth, oBloomRenderRect.m_uHeight );

	for( int i = 0; i < 2; ++i )
	{
		if( oBloomRenderRect.m_uWidth != m_oBloomRT[ i ].GetWidth() || oBloomRenderRect.m_uHeight != m_oBloomRT[ i ].GetHeight() )
		{
			m_oBloomRT[ i ].Destroy();
			m_oBloomRT[ i ].Create( RenderTargetDesc( oBloomRenderRect.m_uWidth, oBloomRenderRect.m_uHeight, TextureFormat::RGB16 ) );
		}
	}

	g_pRenderer->CopyRenderTargetColor( oInput, 1, m_oBloomRT[ 0 ], 0 );

	Technique& oBlurTechnique = m_xBlur->GetTechnique();
	g_pRenderer->SetTechnique( oBlurTechnique );

	TechniqueArrayParameter& oParamKernel = m_oBlurSheet.GetArrayParameter( BlurParam::KERNEL );

	const Array< float > aKernel = ComputeBlurKernel( 16 );
	for( uint u = 0; u < aKernel.Count(); ++u )
		oParamKernel.SetValue( aKernel[ u ], u );

	bool bVertical = false;

	for( int i = 0; i < 2 * m_iIterations; ++i )
	{
		if( bVertical )
		{
			g_pRenderer->SetRenderTarget( m_oBloomRT[ 0 ] );

			glClear( GL_COLOR_BUFFER_BIT );

			m_oBlurSheet.GetParameter( BlurParam::VERTICAL ).SetValue( true );
			m_oBlurSheet.GetParameter( BlurParam::DELTA ).SetValue( 1.f / m_oBloomRT[ 0 ].GetHeight() );

			g_pRenderer->SetTextureSlot( m_oBloomRT[ 1 ].GetColorMap( 0 ), 0 );
			m_oBlurSheet.GetParameter( BlurParam::INPUT_TEXTURE ).SetValue( 0 );
		}
		else
		{
			g_pRenderer->SetRenderTarget( m_oBloomRT[ 1 ] );

			glClear( GL_COLOR_BUFFER_BIT );

			m_oBlurSheet.GetParameter( BlurParam::VERTICAL ).SetValue( false );
			m_oBlurSheet.GetParameter( BlurParam::DELTA ).SetValue( 1.f / m_oBloomRT[ 0 ].GetWidth() );

			g_pRenderer->SetTextureSlot( m_oBloomRT[ 0 ].GetColorMap( 0 ), 0 );
			m_oBlurSheet.GetParameter( BlurParam::INPUT_TEXTURE ).SetValue( 0 );
		}

		g_pRenderer->RenderQuad();

		bVertical = !bVertical;
	}

	g_pRenderer->ClearTextureSlot( 0 );

	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();

	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	g_pRenderer->SetRenderTarget( oOutput );

	g_pRenderer->BlendTextures( oInput.GetColorMap( 0 ), m_oBloomRT[ 0 ].GetColorMap( 0 ) );
}

bool Bloom::OnLoading()
{
	return m_xBlur->IsLoaded();
}

void Bloom::OnLoaded()
{
	m_oBlurSheet.Init( m_xBlur->GetTechnique() );

	m_oBlurSheet.BindArrayParameter( BlurParam::KERNEL, "kernel" );
	m_oBlurSheet.BindParameter( BlurParam::VERTICAL, "vertical" );
	m_oBlurSheet.BindParameter( BlurParam::DELTA, "delta" );
	m_oBlurSheet.BindParameter( BlurParam::INPUT_TEXTURE, "inputTexture" );
}
