#pragma once

#include "Animation.h"
#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Core/stb_truetype.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture.h"

class Resource : public Intrusive
{
public:
	friend class ResourceLoader;

	enum class Status
	{
		LOADING,
		LOADED,
		FAILED
	};

	Resource();
	virtual ~Resource();

	virtual void	Destroy() = 0; // TODO #eric this could probably be a destructor ?

	Status			GetStatus() const;
	bool			IsLoading() const;
	bool			IsLoaded() const;
	bool			IsFailed() const;

protected:
	Status m_eStatus;
};

class FontResource : public Resource
{
public:
	friend class ResourceLoader;

	void							Destroy() override;

	const Texture&					GetAtlas() const;
	const Array< stbtt_packedchar > GetGlyphs() const;

	static const int ATLAS_WIDTH = 512;
	static const int ATLAS_HEIGHT = 512;
	static const int FIRST_GLYPH = 32;
	static const int GLYPH_COUNT = 96;
	static const int FONT_HEIGHT = 16;

private:
	Texture						m_oAtlas;
	Array< stbtt_packedchar >	m_aPackedCharacters;
};

class ShaderResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

	const Shader&	GetShader() const;

private:
	Shader m_oShader;
};

class TechniqueResource : public Resource
{
public:
	friend class ResourceLoader;

	void				Destroy() override;

	Technique&			GetTechnique();
	const Technique&	GetTechnique() const;

private:
	Technique						m_oTechnique;

	Array< StrongPtr< Resource > >	m_aShaderResources;
};

class TextureResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

	const Texture&	GetTexture() const;

private:
	Texture m_oTexture;
};

class ModelResource : public Resource
{
public:
	friend class ResourceLoader;

	void						Destroy() override;

	const Array< Mesh >&		GetMeshes() const;

	const Array< Animation >&	GetAnimations() const;
	const Skeleton&				GetSkeleton() const;

	const Array< glm::mat4x3 >& GetPoseMatrices() const;
	const Array< glm::mat4x3 >& GetSkinMatrices() const;

private:
	Array< Mesh >			m_aMeshes;
	Array< Animation >		m_aAnimations;
	Skeleton				m_oSkeleton;
	Array < glm::mat4x3 >	m_aPoseMatrices;
	Array < glm::mat4x3 >	m_aSkinMatrices;
};

using FontResPtr = StrongPtr< FontResource >;
using ShaderResPtr = StrongPtr< ShaderResource >;
using TechniqueResPtr = StrongPtr< TechniqueResource >;
using TextureResPtr = StrongPtr< TextureResource >;
using ModelResPtr = StrongPtr< ModelResource >;
