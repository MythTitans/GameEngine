#include "Visual.h"

#include "Graphics/Renderer.h"

VisualComponent::VisualComponent( Entity* pEntity )
	: Component( pEntity )
	, m_sModelFile( "" )
{
}

void VisualComponent::Setup( const char* sModelFile )
{
	m_sModelFile = sModelFile;
}

void VisualComponent::Initialize()
{
	m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
	m_xTechnique = g_pResourceLoader->LoadTechnique( "Shader/forward_opaque.tech" );
}

bool VisualComponent::IsInitialized()
{
	return m_xModel->IsLoaded() && m_xTechnique->IsLoaded();
}

void VisualComponent::Update( const float fDeltaTime )
{
	g_pRenderer->m_oVisualStructure.AddNode( GetEntity(), &m_xModel->GetMeshes(), m_xTechnique->GetTechnique() );
}