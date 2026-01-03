#pragma once

#include <typeindex>
#include <unordered_map>

#include "Core/Array.h"
#include "Material.h"

class Technique;

struct MaterialsHolderBase
{
	MaterialsHolderBase();
	virtual ~MaterialsHolderBase();

	virtual void PrepareMaterials( Technique& oTechnique ) = 0;
	virtual void ApplyMaterial( const uint uMaterialID, Technique& oTechnique ) = 0;
};

template < typename MaterialData >
struct MaterialsHolder : MaterialsHolderBase
{
	void PrepareMaterials( Technique& oTechnique ) override
	{
		MaterialData::PrepareMaterial( oTechnique );
	}

	void ApplyMaterial( const uint uMaterialID, Technique& oTechnique ) override
	{
		m_aMaterialData[ uMaterialID ].ApplyMaterial( uMaterialID, oTechnique );
	}

	static const MaterialData& GetDefaultMaterialData()
	{
		static const MaterialData oMaterialData;
		return oMaterialData;
	}

	Array< MaterialData > m_aMaterialData;
};

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	template < typename MaterialData >
	MaterialReference CreateMaterial( const MaterialData& oMaterialData )
	{
		MaterialsHolderBase*& pMaterialsHolderBase = m_mMaterialsHolders[ typeid( MaterialData ) ];
		if( pMaterialsHolderBase == nullptr )
			pMaterialsHolderBase = new MaterialsHolder< MaterialData >;

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( pMaterialsHolderBase );
		pMaterialsHolder->m_aMaterialData.PushBack( oMaterialData );

		return MaterialReference( oMaterialData, pMaterialsHolder->m_aMaterialData.Count() - 1 );
	}

	template < typename MaterialData >
	void UpdateMaterial( const MaterialReference& oMaterialReference, const MaterialData& oMaterialData )
	{
		ASSERT( oMaterialReference.m_oTypeIndex == typeid( MaterialData ) );

		auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
		if( it == m_mMaterialsHolders.end() || it->second == nullptr )
			return;

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( it->second );
		pMaterialsHolder->m_aMaterialData[ oMaterialReference.m_iMaterialID ] = oMaterialData;
	}

	template < typename MaterialData >
	const MaterialData& GetMaterial( const MaterialReference& oMaterialReference )
	{
		ASSERT( oMaterialReference.m_oTypeIndex == typeid( MaterialData ) );

		auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
		if( it == m_mMaterialsHolders.end() || it->second == nullptr )
			return MaterialsHolder< MaterialData >::GetDefaultMaterialData();

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( it->second );
		return pMaterialsHolder->m_aMaterialData[ oMaterialReference.m_iMaterialID ];
	}

	template < typename MaterialData, typename GPUMaterialData >
	void ExportMaterialsToGPU( GPUMaterialData* pGPUMaterials )
	{
		auto it = m_mMaterialsHolders.find( typeid( MaterialData ) );
		if( it == m_mMaterialsHolders.end() || it->second == nullptr )
			return;

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( it->second );

		ASSERT( pMaterialsHolder->m_aMaterialData.Count() <= MAX_MATERIAL_COUNT );
		const uint uMaterialCount = glm::min( pMaterialsHolder->m_aMaterialData.Count(), MAX_MATERIAL_COUNT );

		for( uint u = 0; u < uMaterialCount; ++u )
			pMaterialsHolder->m_aMaterialData[ u ].ExportToGPU( pGPUMaterials[ u ] );
	}

	template < typename MaterialData >
	bool IsMaterialType( const MaterialReference& oMaterialReference )
	{
		return oMaterialReference.m_oTypeIndex == typeid( MaterialData );
	}

	void PrepareMaterials( Technique& oTechnique );
	void ApplyMaterial( const MaterialReference& oMaterialReference, Technique& oTechnique );

private:
	std::unordered_map< std::type_index, MaterialsHolderBase* > m_mMaterialsHolders;
};

extern MaterialManager* g_pMaterialManager;
