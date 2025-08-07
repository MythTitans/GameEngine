#include "MaterialManager.h"

MaterialManager* g_pMaterialManager = nullptr;

MaterialsHolderBase::MaterialsHolderBase()
{
}

MaterialsHolderBase::~MaterialsHolderBase()
{
}

MaterialReference::MaterialReference()
	: m_oTypeIndex( typeid( void ) )
	, m_iMaterialID( -1 )
{
}

MaterialManager::MaterialManager()
{
	g_pMaterialManager = this;
}

MaterialManager::~MaterialManager()
{
	g_pMaterialManager = nullptr;
}

void MaterialManager::PrepareMaterials( Technique& oTechnique )
{
	for( const auto& oPair : m_mMaterialsHolders )
		oPair.second->PrepareMaterials( oTechnique );
}

void MaterialManager::ApplyMaterial( const MaterialReference& oMaterialReference, Technique& oTechnique )
{
	auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
	if( it == m_mMaterialsHolders.end() || it->second == nullptr )
		return;

	it->second->ApplyMaterial( oMaterialReference.m_iMaterialID, oTechnique );
}
