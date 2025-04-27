#include "Visual.h"

#include "Animator.h"
#include "Editor/EditorHelpers.h"
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

void VisualComponent::DisplayInspector()
{
	if( ImGui::CollapsingHeader( "Material" ) )
	{
		for( const Mesh& oMesh : GetMeshes() )
		{
			if( g_pMaterialManager->IsMaterialType< LitMaterialData >( oMesh.GetMaterial() ) )
			{
				LitMaterialData oMaterialData = g_pMaterialManager->GetMaterial< LitMaterialData >( oMesh.GetMaterial() );
				ColorEdit( "Diffuse color", oMaterialData.m_vDiffuseColor );
				ColorEdit( "Specular color", oMaterialData.m_vSpecularColor );
				ColorEdit( "Emissive color", oMaterialData.m_vEmissiveColor );
				ImGui::DragFloat( "Shininess", &oMaterialData.m_fShininess );
				TexturePreview( "Diffuse map", oMaterialData.m_xDiffuseTextureResource.GetPtr() );
				TexturePreview( "Normal map", oMaterialData.m_xNormalTextureResource.GetPtr() );
				TexturePreview( "Specular map", oMaterialData.m_xSpecularTextureResource.GetPtr() );
				TexturePreview( "Emissive map", oMaterialData.m_xEmissiveTextureResource.GetPtr() );
				g_pMaterialManager->UpdateMaterial( oMesh.GetMaterial(), oMaterialData );
			}
		}
	}
}

const Array< Mesh >& VisualComponent::GetMeshes() const
{
	return m_xModel->GetMeshes();
}
