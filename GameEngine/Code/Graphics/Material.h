#pragma once

#include <glm/glm.hpp>
#include <typeindex>

#include "Game/ResourceTypes.h"
#include "Graphics/Technique.h"

inline constexpr uint MAX_MATERIAL_COUNT = 128;

class Technique;
class TextureResource;

struct GPULitMaterialData
{
	alignas( 16 ) glm::vec3	m_vDiffuseColor;
	alignas( 16 ) glm::vec3	m_vSpecularColor;
	alignas( 16 ) glm::vec3	m_vEmissiveColor;
	alignas( 4 ) float		m_fShininess;
};

struct GPUUnlitMaterialData
{
	alignas( 16 ) glm::vec3	m_vDiffuseColor;
};

struct GPUMaterialDataBlock
{
	GPULitMaterialData		m_aLitMaterialData[ MAX_MATERIAL_COUNT ];
	GPUUnlitMaterialData	m_aUnlitMaterialData[ MAX_MATERIAL_COUNT ];
};

struct LitMaterialData
{
	LitMaterialData();

	static void PrepareMaterial( Technique& oTechnique );

	void		ApplyMaterial( const uint uMaterialID, Technique& oTechnique );

	void		ExportToGPU( GPULitMaterialData& oMaterialData ) const;

	enum class LitMaterialParam
	{
		MATERIAL_ID,
		DIFFUSE_MAP,
		NORMAL_MAP,
		SPECULAR_MAP,
		EMISSIVE_MAP,
		_COUNT
	};

	glm::vec3								m_vDiffuseColor;
	glm::vec3								m_vSpecularColor;
	glm::vec3								m_vEmissiveColor;
	float									m_fShininess;

	TextureResPtr							m_xDiffuseTextureResource;
	TextureResPtr							m_xNormalTextureResource;
	TextureResPtr							m_xSpecularTextureResource;
	TextureResPtr							m_xEmissiveTextureResource;

	static PARAM_SHEET( LitMaterialParam )	s_oMaterialSheet;
};

struct UnlitMaterialData
{
	UnlitMaterialData();

	static void PrepareMaterial( Technique& oTechnique );

	void		ApplyMaterial( const uint uMaterialID, Technique& oTechnique );

	void		ExportToGPU( GPUUnlitMaterialData& oMaterialData ) const;

	enum class UnlitMaterialParam
	{
		MATERIAL_ID,
		DIFFUSE_MAP,
		_COUNT
	};

	glm::vec3									m_vDiffuseColor;

	TextureResPtr								m_xDiffuseTextureResource;

	static PARAM_SHEET( UnlitMaterialParam )	s_oMaterialSheet;
};

// TODO #eric may be intrusive and stored in material manager for lifetime management
class MaterialReference
{
public:
	friend class MaterialManager;

	MaterialReference();

	template < typename MaterialData >
	MaterialReference( const MaterialData& /*oMaterialData*/, const uint uMaterialID )
		: m_oTypeIndex( typeid( MaterialData ) )
		, m_iMaterialID( uMaterialID )
	{
	}

private:
	std::type_index m_oTypeIndex;
	int				m_iMaterialID;
};
