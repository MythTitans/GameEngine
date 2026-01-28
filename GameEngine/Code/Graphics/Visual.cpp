#include "Visual.h"

#include "Editor/Inspector.h"
#include "Game/Entity.h"
#include "Game/GameContext.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/MaterialManager.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"
#include "Physics/Rigidbody.h"

REGISTER_COMPONENT( VisualComponent );
SET_COMPONENT_PRIORITY_AFTER( VisualComponent, RigidbodyComponent );

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
	return m_xModel->IsLoading() == false && m_xTechnique->IsLoaded();
}

void VisualComponent::Start()
{
	const Entity* pEntity = GetEntity();

	m_pVisualNode = g_pRenderer->m_oVisualStructure.AddVisual( pEntity, m_xTechnique->GetTechnique() );
	m_pVisualNode->m_aMeshes = m_xModel->GetMeshes();

	m_oModelAABB = m_xModel->GetAABB();
}

void VisualComponent::Update( const GameContext& oGameContext )
{
	if( oGameContext.m_bEditing && m_xModel->IsLoaded() )
	{
		m_pVisualNode->m_aMeshes = m_xModel->GetMeshes();
		m_oModelAABB = m_xModel->GetAABB();
	}

	m_pVisualNode->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTRS();
	m_pVisualNode->m_oAABB = AxisAlignedBox::FromOrientedBox( OrientedBox::FromAxisAlignedBox( m_oModelAABB, m_pVisualNode->m_mMatrix ) );
}

void VisualComponent::Stop()
{
	g_pRenderer->m_oVisualStructure.RemoveVisual( m_pVisualNode );
}

void VisualComponent::Dispose()
{
	m_xModel = nullptr;
	m_xTechnique = nullptr;
}

void VisualComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected && m_pVisualNode != nullptr )
		g_pDebugDisplay->DisplayWireAxisBox( m_pVisualNode->m_oAABB.m_vMin, m_pVisualNode->m_oAABB.m_vMax, glm::vec3( 1.f, 0.f, 1.f ) );
}

#ifdef EDITOR
bool VisualComponent::DisplayInspector()
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

	return false;
}

void VisualComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Model" )
		m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
}
#endif

const Array< Mesh >& VisualComponent::GetMeshes() const
{
	return m_xModel->GetMeshes();
}
