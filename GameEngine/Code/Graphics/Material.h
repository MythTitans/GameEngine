#pragma once

#include <glm/glm.hpp>

#include "Game/ResourceTypes.h"
#include "Graphics/Technique.h"

class Technique;
class TextureResource;

struct LitMaterialData
{
	LitMaterialData();

	static void PrepareMaterial( Technique& oTechnique );

	void		ApplyMaterial( Technique& oTechnique );

	enum class LitMaterialParam
	{
		DIFFUSE_COLOR,
		SPECULAR_COLOR,
		EMISSIVE_COLOR,
		SHININESS,
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

	void		ApplyMaterial( Technique& oTechnique );

	enum class UnlitMaterialParam
	{
		DIFFUSE_COLOR,
		DIFFUSE_MAP,
		_COUNT
	};

	glm::vec3									m_vDiffuseColor;

	TextureResPtr								m_xDiffuseTextureResource;

	static PARAM_SHEET( UnlitMaterialParam )	s_oMaterialSheet;
};
