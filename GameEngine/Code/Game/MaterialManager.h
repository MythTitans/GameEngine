#pragma once

#include <unordered_map>

#include "Core/Array.h"
#include "Core/Intrusive.h"

class Technique;

struct MaterialsHolderBase
{
	MaterialsHolderBase();
	virtual ~MaterialsHolderBase();

	virtual void ApplyMaterial( const uint uMaterialID, Technique& oTechnique ) = 0;
};

template < typename MaterialData >
struct MaterialsHolder : MaterialsHolderBase
{
	void ApplyMaterial( const uint uMaterialID, Technique& oTechnique ) override
	{
		m_aMaterialData[ uMaterialID ].ApplyMaterial( oTechnique );
	}

	static const MaterialData& GetDefaultMaterialData()
	{
		static const MaterialData oMaterialData;
		return oMaterialData;
	}

	Array< MaterialData > m_aMaterialData;
};

// TODO #eric may be intrusive and stored in material manager for lifetime management
class MaterialReference
{
public:
	friend class MaterialManager;

	MaterialReference();

	template < typename MaterialData >
	MaterialReference( const MaterialData& /*oMaterialData*/, const uint uMaterialID )
		: m_oTypeIndex( std::type_index( typeid( MaterialData ) ) )
		, m_iMaterialID( uMaterialID )
	{
	}

	int GetMaterialID() const;

private:
	std::type_index m_oTypeIndex;
	int				m_iMaterialID;
};

class MaterialManager
{
public:
	MaterialManager();
	~MaterialManager();

	template < typename MaterialData >
	MaterialReference CreateMaterial( const MaterialData& oMaterialData )
	{
		MaterialsHolderBase*& pMaterialsHolderBase = m_mMaterialsHolders[ std::type_index( typeid( MaterialData ) ) ];
		if( pMaterialsHolderBase == nullptr )
			pMaterialsHolderBase = new MaterialsHolder< MaterialData >;

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( pMaterialsHolderBase );
		pMaterialsHolder->m_aMaterialData.PushBack( oMaterialData );

		return MaterialReference( oMaterialData, pMaterialsHolder->m_aMaterialData.Count() - 1 );
	}

	template < typename MaterialData >
	void UpdateMaterial( const MaterialReference& oMaterialReference, const MaterialData& oMaterialData )
	{
		ASSERT( oMaterialReference.m_oTypeIndex == std::type_index( typeid( MaterialData ) ) );

		auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
		if( it == m_mMaterialsHolders.end() || it->second == nullptr )
			return;

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( it->second );
		pMaterialsHolder->m_aMaterialData[ oMaterialReference.m_iMaterialID ] = oMaterialData;
	}

	template < typename MaterialData >
	const MaterialData& GetMaterial( const MaterialReference& oMaterialReference )
	{
		ASSERT( oMaterialReference.m_oTypeIndex == std::type_index( typeid( MaterialData ) ) );

		auto it = m_mMaterialsHolders.find( oMaterialReference.m_oTypeIndex );
		if( it == m_mMaterialsHolders.end() || it->second == nullptr )
			return MaterialsHolder< MaterialData >::GetDefaultMaterialData();

		MaterialsHolder< MaterialData >* pMaterialsHolder = static_cast< MaterialsHolder< MaterialData >* >( it->second );
		return pMaterialsHolder->m_aMaterialData[ oMaterialReference.m_iMaterialID ];
	}

	void ApplyMaterial( const MaterialReference& oMaterialReference, Technique& oTechnique );

private:
	std::unordered_map< std::type_index, MaterialsHolderBase* > m_mMaterialsHolders;
};

extern MaterialManager* g_pMaterialManager;
