#pragma once

#include <mutex>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <nlohmann/json.hpp>

#include "Core/Types.h"
#include "Graphics/Material.h"
#include "ResourceTypes.h"

enum class LoadCommandStatus : uint8
{
	PENDING,
	LOADING,
	LOADED,
	FINISHED,
	WAITING_DEPENDENCIES,
	NOT_FOUND,
	ERROR_READING
};

// This class is not polymorphic, it just defines an interface for subclasses to follow
template < typename Res >
struct LoadCommand
{
	LoadCommand( const char* sFilePath, const StrongPtr< Res >& xResource )
		: m_sFilePath( sFilePath )
		, m_xResource( xResource )
		, m_eStatus( LoadCommandStatus::PENDING )
	{}

	std::string GetFilePath() const
	{
		return std::format( "Data/{}", m_sFilePath );
	}

	bool HasDependencies() const
	{
		return m_aDependencies.Empty() == false;
	}

	bool AnyDependencyFailed() const
	{
		for( const StrongPtr< Resource >& xDependency : m_aDependencies )
		{
			if( xDependency->IsFailed() )
				return true;
		}

		return false;
	}

	bool AllDependenciesLoaded() const
	{
		for( const StrongPtr< Resource >& xDependency : m_aDependencies )
		{
			if( xDependency->IsLoaded() == false )
				return false;
		}

		return true;
	}

	virtual void Load( std::unique_lock< std::mutex >& oLock ) = 0;
	virtual void OnFinished() = 0;
	virtual void OnDependenciesReady() = 0;

	std::string						m_sFilePath;
	StrongPtr< Res >				m_xResource;
	LoadCommandStatus				m_eStatus;
	Array< StrongPtr< Resource > >	m_aDependencies;
};

struct FontLoadCommand : LoadCommand< FontResource >
{
	FontLoadCommand( const char* sFilePath, const FontResPtr& xResource );

	void Load( std::unique_lock< std::mutex >& oLock ) override;
	void OnFinished() override;
	void OnDependenciesReady() override;

	Array< uint8 >				m_aAtlasData;
	Array< stbtt_packedchar >	m_aPackedCharacters;
};

struct TextureLoadCommand : LoadCommand< TextureResource >
{
	TextureLoadCommand( const char* sFilePath, const TextureResPtr& xResource, const bool bSRGB, const bool bUse16Bits );

	void Load( std::unique_lock< std::mutex >& oLock ) override;
	void OnFinished() override;
	void OnDependenciesReady() override;

	int		m_iWidth;
	int		m_iHeight;
	int		m_iDepth;
	bool	m_bSRGB;
	bool	m_bUse16Bits;
	uint8* m_pData;
};

struct ModelLoadCommand : LoadCommand< ModelResource >
{
	ModelLoadCommand( const char* sFilePath, const ModelResPtr& xResource );

	void			Load( std::unique_lock< std::mutex >& oLock ) override;
	void			OnFinished() override;
	void			OnDependenciesReady() override;

	void			LoadAnimations();
	void			LoadSkeleton();
	void			LoadMaterials();
	void			LoadMeshes();
	uint			CountMeshes( aiNode* pNode );
	void			LoadMeshes( aiNode* pNode );
	void			LoadMesh( aiMesh* pMesh );
	void			LoadSkeleton( aiNode* pNode, Skeleton& oParent );
	TextureResPtr	LoadTexture( const std::string& sFileName, const bool bSRGB = false );
	uint			FetchNodeIndex( const std::string& sName );

	aiScene* m_pScene;
	Array< LitMaterialData >				m_aMaterials;
	std::unordered_map< std::string, uint > m_mNodeIndices;
};

struct ShaderLoadCommand : LoadCommand< ShaderResource >
{
	ShaderLoadCommand( const char* sFilePath, const ShaderResPtr& xResource, Array< std::string >&& aFlags );

	void Load( std::unique_lock< std::mutex >& oLock ) override;
	void OnFinished() override;
	void OnDependenciesReady() override;

	std::string				m_sShaderCode;
	ShaderType				m_eShaderType;
	Array< std::string >	m_aFlags;
};

struct TechniqueLoadCommand : LoadCommand< TechniqueResource >
{
	TechniqueLoadCommand( const char* sFilePath, const TechniqueResPtr& xResource );

	void Load( std::unique_lock< std::mutex >& oLock ) override;
	void OnFinished() override;
	void OnDependenciesReady() override;

	Array< std::string >					m_aParameters;
	Array< std::pair< std::string, uint > > m_aArrayParameters;
	Array< std::string >					m_aShaders;
};

struct MaterialLoadCommand : LoadCommand< MaterialResource >
{
	MaterialLoadCommand( const char* sFilePath, const MaterialResPtr& xResource );

	void Load( std::unique_lock< std::mutex >& oLock ) override;
	void OnFinished() override;
	void OnDependenciesReady() override;

	void LoadMaterial();

	std::string		m_sType;
	nlohmann::json	m_oJsonData;
};
