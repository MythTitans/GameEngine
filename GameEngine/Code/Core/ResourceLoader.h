#pragma once

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <assimp/Importer.hpp>

#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture.h"

struct aiMesh;
struct aiNode;
struct aiScene;

class Resource : public Intrusive
{
public:
	friend class ResourceLoader;

	Resource();

	virtual void	Destroy() = 0;

	bool			IsLoaded() const;

protected:
	bool m_bLoaded;
};

class TextureResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

	Texture&		GetTexture();
	const Texture&	GetTexture() const;

private:
	Texture m_oTexture;
};

using TextureResPtr = StrongPtr< TextureResource >;

class ModelResource : public Resource
{
public:
	friend class ResourceLoader;

	void					Destroy() override;

	Array< Mesh >&			GetMeshes();
	const Array< Mesh >&	GetMeshes() const;

private:
	Array< Mesh > m_aMeshes;
};

using ModelResPtr = StrongPtr< ModelResource >;

class ShaderResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

	Shader&			GetShader();
	const Shader&	GetShader() const;

private:
	Shader m_oShader;
};

using ShaderResPtr = StrongPtr< ShaderResource >;

class TechniqueResource : public Resource
{
public:
	friend class ResourceLoader;

	void				Destroy() override;

	Technique&			GetTechnique();
	const Technique&	GetTechnique() const;

private:
	Technique				m_oTechnique;

	Array< ShaderResPtr >	m_aShaderResources;
};

using TechniqueResPtr = StrongPtr< TechniqueResource >;

class ResourceLoader
{
public:
	template < typename LoadCommand >
	friend void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName );

	template < typename LoadCommand >
	friend uint CheckFinishedProcessingLoadCommands( Array< LoadCommand >& aLoadCommands );

	ResourceLoader();
	~ResourceLoader();

	TextureResPtr	LoadTexture( const std::filesystem::path& oFilePath );
	ModelResPtr		LoadModel( const std::filesystem::path& oFilePath );
	ShaderResPtr	LoadShader( const std::filesystem::path& oFilePath );
	TechniqueResPtr LoadTechnique( const std::filesystem::path& oFilePath );

	void			PreUpdate();
	void			Update();
	void			PostUpdate();

private:
	void			Load();
	void			ProcessPendingLoadCommands();
	void			CheckFinishedProcessingLoadCommands();
	void			DestroyUnusedResources();

	enum class LoadCommandStatus : uint8
	{
		PENDING,
		LOADING,
		LOADED,
		FINISHED,
		NOT_FOUND,
		ERROR_READING
	};

	template < typename Resource >
	struct LoadCommand
	{
		LoadCommand( const std::filesystem::path& oFilePath, const StrongPtr< Resource >& xResource )
			: m_oFilePath( oFilePath )
			, m_xResource( xResource )
			, m_eStatus( LoadCommandStatus::PENDING )
		{
		}

		std::filesystem::path	m_oFilePath;
		StrongPtr< Resource >	m_xResource;
		LoadCommandStatus		m_eStatus;
	};

	struct TextureLoadCommand : LoadCommand< TextureResource >
	{
		TextureLoadCommand( const std::filesystem::path& oFilePath, const TextureResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock );
		void OnLoaded();

		int		m_iWidth;
		int		m_iHeight;
		int		m_iDepth;
		uint8*	m_pData;
	};

	struct ModelLoadCommand : LoadCommand< ModelResource >
	{
		ModelLoadCommand( const std::filesystem::path& oFilePath, const ModelResPtr& xResource, Assimp::Importer& oModelImporter );

		void Load( std::unique_lock< std::mutex >& oLock );
		void OnLoaded();

		uint CountMeshes( aiNode* pNode );
		void LoadMeshes( aiNode* pNode );
		void LoadMesh( aiMesh* pMesh );

		Assimp::Importer&	m_oModelImporter;
		aiScene*			m_pScene;
	};

	struct ShaderLoadCommand : LoadCommand< ShaderResource >
	{
		ShaderLoadCommand( const std::filesystem::path& oFilePath, const ShaderResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock );
		void OnLoaded();

		std::string m_sShaderCode;
		ShaderType	m_eShaderType;
	};

	struct TechniqueLoadCommand : LoadCommand< TechniqueResource >
	{
		TechniqueLoadCommand( const std::filesystem::path& oFilePath, const TechniqueResPtr& xResource );

		void OnLoaded();

		Array< ShaderResPtr > m_aShaderResources;
	};

	struct LoadCommands
	{
		Array< TextureLoadCommand >		m_aTextureLoadCommands;
		Array< ModelLoadCommand >		m_aModelLoadCommands;
		Array< ShaderLoadCommand >		m_aShaderLoadCommands;
		Array< TechniqueLoadCommand >	m_aTechniqueLoadCommands;

		uint Count() const;
		bool Empty() const;
		void Grab( LoadCommands& oLoadCommands );
		void Clear();
	};

	using TextureResourceMap = std::unordered_map< std::filesystem::path, TextureResPtr >;
	using ModelResourceMap = std::unordered_map< std::filesystem::path, ModelResPtr >;
	using ShaderResourceMap = std::unordered_map< std::filesystem::path, ShaderResPtr >;
	using TechniqueResourceMap = std::unordered_map< std::filesystem::path, TechniqueResPtr >;

	TextureResourceMap		m_mTextureResources;
	ModelResourceMap		m_mModelResources;
	ShaderResourceMap		m_mShaderResources;
	TechniqueResourceMap	m_mTechniqueResources;

	LoadCommands			m_oPendingLoadCommands;
	LoadCommands			m_oProcessingLoadCommands;

	std::atomic_bool		m_bRunning;

	std::mutex				m_oProcessingCommandsMutex;
	std::condition_variable m_oProcessingCommandsConditionVariable;
	std::jthread			m_oIOThread;

	Assimp::Importer		m_oModelImporter;
};

extern ResourceLoader* g_pResourceLoader;