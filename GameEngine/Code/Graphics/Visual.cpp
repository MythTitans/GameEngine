#include "Visual.h"

#include <glm/gtc/matrix_inverse.hpp>

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
	, m_bModelDirty( false )
	, m_bMaterialDirty( false )
{
}

void VisualComponent::Setup( const char* sModelFile )
{
	m_sModelFile = sModelFile;
	m_sMaterialFile = "";
}

void VisualComponent::Initialize()
{
	m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
	m_xMaterial = m_sMaterialFile.empty() ? nullptr : g_pResourceLoader->LoadMaterial( m_sMaterialFile.c_str() );
	m_xTechnique = g_pResourceLoader->LoadTechnique( "Shader/forward_opaque.tech" );
}

bool VisualComponent::IsInitialized() const
{
	return m_xModel->IsLoading() == false && m_xTechnique->IsLoaded() && ( m_xMaterial == nullptr || m_xMaterial->IsLoaded() );
}

void VisualComponent::Start()
{
	const Entity* pEntity = GetEntity();

	m_pVisualNode = g_pRenderer->m_oVisualStructure.AddVisual( pEntity, m_xTechnique->GetTechnique() );

	UpdateModel();
	m_pVisualNode->UpdateTransformAndAABB( pEntity->GetWorldTransform(), m_oModelAABB );
}

void VisualComponent::Update( const GameContext& oGameContext )
{
	const Entity* pEntity = GetEntity();

	if( m_bModelDirty && m_xModel->IsLoaded() )
		UpdateModel();

	if( m_bMaterialDirty && m_xMaterial != nullptr && m_xMaterial->IsLoaded() )
		UpdateMaterial();

	if( pEntity->IsDirty() )
		m_pVisualNode->UpdateTransformAndAABB( pEntity->GetWorldTransform(), m_oModelAABB );
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
		if( m_xMaterial != nullptr && m_xMaterial->IsLoaded() )
		{
			MaterialReference oMaterial = m_xMaterial->GetMaterial();
			if( g_pMaterialManager->IsMaterialType< LitMaterialData >( m_xMaterial->GetMaterial() ) )
			{
				LitMaterialData oMaterialData = g_pMaterialManager->GetMaterial< LitMaterialData >( oMaterial );
				ColorEdit( "Diffuse color", oMaterialData.m_oDiffuseColor );
				ColorEdit( "Specular color", oMaterialData.m_oSpecularColor );
				ColorEdit( "Emissive color", oMaterialData.m_oEmissiveColor );
				ImGui::DragFloat( "Shininess", &oMaterialData.m_fShininess );
				TexturePreview( "Diffuse map", oMaterialData.m_xDiffuseTextureResource.GetPtr() );
				TexturePreview( "Normal map", oMaterialData.m_xNormalTextureResource.GetPtr() );
				TexturePreview( "Specular map", oMaterialData.m_xSpecularTextureResource.GetPtr() );
				TexturePreview( "Emissive map", oMaterialData.m_xEmissiveTextureResource.GetPtr() );
				g_pMaterialManager->UpdateMaterial( oMaterial, oMaterialData );
			}
		}
		else
		{
			for( const Mesh& oMesh : GetMeshes() )
			{
				MaterialReference oMaterial = oMesh.GetMaterial();
				if( g_pMaterialManager->IsMaterialType< LitMaterialData >( oMaterial ) )
				{
					LitMaterialData oMaterialData = g_pMaterialManager->GetMaterial< LitMaterialData >( oMaterial );
					ColorEdit( "Diffuse color", oMaterialData.m_oDiffuseColor );
					ColorEdit( "Specular color", oMaterialData.m_oSpecularColor );
					ColorEdit( "Emissive color", oMaterialData.m_oEmissiveColor );
					ImGui::DragFloat( "Shininess", &oMaterialData.m_fShininess );
					TexturePreview( "Diffuse map", oMaterialData.m_xDiffuseTextureResource.GetPtr() );
					TexturePreview( "Normal map", oMaterialData.m_xNormalTextureResource.GetPtr() );
					TexturePreview( "Specular map", oMaterialData.m_xSpecularTextureResource.GetPtr() );
					TexturePreview( "Emissive map", oMaterialData.m_xEmissiveTextureResource.GetPtr() );
					g_pMaterialManager->UpdateMaterial( oMaterial, oMaterialData );
				}
			}
		}
	}

	return false;
}

void VisualComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Model" )
	{
		m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
		m_bModelDirty = true;
	}
	else if( sProperty == "Material" )
	{
		m_xMaterial = m_sMaterialFile.empty() ? nullptr : g_pResourceLoader->LoadMaterial( m_sMaterialFile.c_str() );
		m_bMaterialDirty = true;
	}
}
#endif

const Array< Mesh >& VisualComponent::GetMeshes() const
{
	return m_xModel->GetMeshes();
}

void VisualComponent::UpdateModel()
{
	m_pVisualNode->m_aMeshes = m_xModel->GetMeshes();
	m_oModelAABB = m_xModel->GetAABB();

	m_bModelDirty = false;
}

void VisualComponent::UpdateMaterial()
{
	for( Mesh& oMesh : m_pVisualNode->m_aMeshes )
		oMesh.SetMaterial( m_xMaterial->GetMaterial() );

	m_bMaterialDirty = false;
}
