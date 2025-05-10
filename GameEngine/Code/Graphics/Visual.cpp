#include "Visual.h"

#include "Editor/Inspector.h"
#include "Game/Animator.h"
#include "Game/GameEngine.h"
#include "Game/Entity.h"
#include "Graphics/Renderer.h"

REGISTER_COMPONENT( VisualComponent );

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
	m_hAnimatorComponent = GetComponent< AnimatorComponent >();
}

bool VisualComponent::IsInitialized() const
{
	return m_xModel->IsLoaded() && m_xTechnique->IsLoaded();
}

void VisualComponent::Update( const GameContext& oGameContext )
{
	if( m_xModel->IsLoaded() == false )
		return;

	if( oGameContext.m_bEditing )
		m_hAnimatorComponent = GetComponent< AnimatorComponent >();

	const Entity* pEntity = GetEntity();

	const Array< glm::mat4 > aEmpty;
	const Array< glm::mat4 >& aBoneMatrices = m_hAnimatorComponent.IsValid() ? m_hAnimatorComponent->GetBoneMatrices() : aEmpty;
	g_pRenderer->m_oVisualStructure.AddNode( pEntity, pEntity->GetWorldTransform().GetMatrixTRS(), m_xModel->GetMeshes(), aBoneMatrices, m_xTechnique->GetTechnique() );
}

void VisualComponent::Dispose()
{
	m_xModel = nullptr;
	m_xTechnique = nullptr;
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

void VisualComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Model" )
		m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
}

const Array< Mesh >& VisualComponent::GetMeshes() const
{
	return m_xModel->GetMeshes();
}
