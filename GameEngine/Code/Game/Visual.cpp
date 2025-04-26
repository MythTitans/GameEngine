#include "Visual.h"

#include "Animator.h"
#include "Entity.h"
#include "Graphics/Renderer.h"

VisualComponent::VisualComponent( Entity* pEntity )
	: Component( pEntity )
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
	const Entity* pEntity = GetEntity();

	const AnimatorComponent* pAnimatorComponent = g_pComponentManager->GetComponent< AnimatorComponent >( pEntity );
	const Array< Mesh >* pMeshes = &m_xModel->GetMeshes();
	const Array< glm::mat4 >* pBoneMatrices = pAnimatorComponent != nullptr ? &pAnimatorComponent->GetBoneMatrices() : nullptr;
	g_pRenderer->m_oVisualStructure.AddNode( pEntity, pEntity->GetWorldTransform().GetMatrixTRS(), pMeshes, pBoneMatrices, m_xTechnique->GetTechnique() );
}

const Array< Mesh >& VisualComponent::GetMeshes() const
{
	return m_xModel->GetMeshes();
}
