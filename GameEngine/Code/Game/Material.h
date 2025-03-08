#pragma once

#include <glm/glm.hpp>

#include "Core/Intrusive.h"
#include "Graphics/Texture.h"

class Technique;
class TextureResource;

using TextureResPtr = StrongPtr< TextureResource >;

struct LitMaterialData
{
	LitMaterialData();

	void ApplyMaterial( Technique& oTechnique );

	glm::vec3		m_vDiffuseColor;

	TextureResPtr	m_xDiffuseTextureResource;
	TextureResPtr	m_xNormalTextureResource;
};

struct UnlitMaterialData
{
	UnlitMaterialData();

	void ApplyMaterial( Technique& oTechnique );

	glm::vec3		m_vDiffuseColor;

	TextureResPtr	m_xDiffuseTextureResource;
};
