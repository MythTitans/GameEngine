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
}

bool VisualComponent::IsInitialized()
{
	return m_xModel->IsLoaded();
}

void VisualComponent::Update( const float fDeltaTime )
{
	g_pRenderer->m_oVisualStructure.AddNode( GetEntity(), &m_xModel->GetMeshes(), g_pRenderer->GetForwardOpaque() );
}