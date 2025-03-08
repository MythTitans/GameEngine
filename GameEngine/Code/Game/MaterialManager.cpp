#include "MaterialManager.h"

MaterialManager* g_pMaterialManager = nullptr;

MaterialsHolderBase::MaterialsHolderBase()
{
}

MaterialsHolderBase::~MaterialsHolderBase()
{
}

MaterialReference::MaterialReference()
	: m_oTypeIndex( std::type_index( typeid( void ) ) )
	, m_iMaterialID( -1 )
{
}

int MaterialReference::GetMaterialID() const
{
	return m_iMaterialID;
}

MaterialManager::MaterialManager()
{
	g_pMaterialManager = this;
}

MaterialManager::~MaterialManager()
{
	g_pMaterialManager = nullptr;
}

void MaterialManager::ApplyMaterial( const MaterialReference& oMaterialReference, Technique& oTechnique )
{
	auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
	if( it == m_mMaterialsHolders.end() || it->second == nullptr )
		return;

	it->second->ApplyMaterial( oMaterialReference.m_iMaterialID, oTechnique );
}
