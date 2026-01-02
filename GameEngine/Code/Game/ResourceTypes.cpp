#include "ResourceTypes.h"

#include "Graphics/Mesh.h"

Resource::Resource()
	: m_eStatus( Status::LOADING )
{
}

Resource::~Resource()
{
}

Resource::Status Resource::GetStatus() const
{
	return m_eStatus;
}

bool Resource::IsLoading() const
{
	return m_eStatus == Status::LOADING;
}

bool Resource::IsLoaded() const
{
	return m_eStatus == Status::LOADED;
}

bool Resource::IsFailed() const
{
	return m_eStatus == Status::FAILED;
}

uint64 FontResource::GetSize() const
{
	return sizeof( FontResource );
}

void FontResource::Destroy()
{
	m_oAtlas.Destroy();
}

const Texture& FontResource::GetAtlas() const
{
	return m_oAtlas;
}

const Array< stbtt_packedchar > FontResource::GetGlyphs() const
{
	return m_aPackedCharacters;
}

uint64 TextureResource::GetSize() const
{
	return sizeof( TextureResource );
}

void TextureResource::Destroy()
{
	m_oTexture.Destroy();
}

const Texture& TextureResource::GetTexture() const
{
	return m_oTexture;
}

uint64 ModelResource::GetSize() const
{
	return sizeof( ModelResource );
}

void ModelResource::Destroy()
{
	for( Mesh& oMesh : m_aMeshes )
		oMesh.Destroy();
}

const Array< Mesh >& ModelResource::GetMeshes() const
{
	return m_aMeshes;
}

const Array< Animation >& ModelResource::GetAnimations() const
{
	return m_aAnimations;
}

const Skeleton& ModelResource::GetSkeleton() const
{
	return m_oSkeleton;
}

const Array< glm::mat4x3 >& ModelResource::GetPoseMatrices() const
{
	return m_aPoseMatrices;
}

const Array< glm::mat4x3 >& ModelResource::GetSkinMatrices() const
{
	return m_aSkinMatrices;
}

uint64 ShaderResource::GetSize() const
{
	return sizeof( ShaderResource );
}

void ShaderResource::Destroy()
{
	m_oShader.Destroy();
}

const Shader& ShaderResource::GetShader() const
{
	return m_oShader;
}

uint64 TechniqueResource::GetSize() const
{
	return sizeof( TechniqueResource );
}

void TechniqueResource::Destroy()
{
	m_oTechnique.Destroy();
}

Technique& TechniqueResource::GetTechnique()
{
	return m_oTechnique;
}

const Technique& TechniqueResource::GetTechnique() const
{
	return m_oTechnique;
}