#include "Visual.h"

#include "Editor/Inspector.h"
#include "Game/Animator.h"
#include "Game/GameEngine.h"
#include "Game/Entity.h"
#include "Graphics/Renderer.h"

REGISTER_COMPONENT( VisualComponent );

VisualComponent::VisualComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pVisualNode( nullptr )
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

bool VisualComponent::IsInitialized() const
{
	return m_xModel->IsLoaded() && m_xTechnique->IsLoaded();
}

void VisualComponent::Start()
{
	const Entity* pEntity = GetEntity();

	m_pVisualNode = g_pRenderer->m_oVisualStructure.AddNode( pEntity, m_xTechnique->GetTechnique() );
	m_pVisualNode->m_aMeshes = m_xModel->GetMeshes();
}

void VisualComponent::Update( const GameContext& oGameContext )
{
	if( oGameContext.m_bEditing && m_xModel->IsLoaded() )
		m_pVisualNode->m_aMeshes = m_xModel->GetMeshes();

	const Entity* pEntity = GetEntity();

	m_pVisualNode->m_mMatrix = pEntity->GetWorldTransform().GetMatrixTRS();
}

void VisualComponent::Stop()
{
	g_pRenderer->m_oVisualStructure.RemoveNode( m_pVisualNode );
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
