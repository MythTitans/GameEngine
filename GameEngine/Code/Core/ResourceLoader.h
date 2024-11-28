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
#include "Graphics/Texture.h"

struct aiMesh;
struct aiNode;
struct aiScene;

class Resource : public Intrusive
{
public:
	friend class ResourceLoader;

	Resource();

	bool			IsLoaded() const;

protected:
	bool m_bLoaded;
};

class TextureResource : public Resource
{
public:
	friend class ResourceLoader;

	Texture&		GetTexture();
	const Texture&	GetTexture() const;

private:
	Texture m_oTexture;
};

class ModelResource : public Resource
{
public:
	friend class ResourceLoader;

	Array< Mesh >&			GetMeshes();
	const Array< Mesh >&	GetMeshes() const;

private:
	Array< Mesh > m_aMeshes;
};

class ResourceLoader
{
public:
	ResourceLoader();

	StrongPtr< TextureResource >	LoadTexture( const std::filesystem::path& oFilePath );
	StrongPtr< ModelResource >		LoadModel( const std::filesystem::path& oFilePath );

	void							PreUpdate();
	void							Update();
	void							PostUpdate();

private:
	void							Load();
	void							ProcessPendingLoadCommands();
	void							CheckFinishedProcessingLoadCommands();
	void							DestroyUnusedResources();

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
		TextureLoadCommand( const std::filesystem::path& oFilePath, const StrongPtr< TextureResource >& xResource );

		void OnLoaded();

		int		m_iWidth;
		int		m_iHeight;
		int		m_iDepth;
		uint8*	m_pData;
	};

	struct ModelLoadCommand : LoadCommand< ModelResource >
	{
		ModelLoadCommand( const std::filesystem::path& oFilePath, const StrongPtr< ModelResource >& xResource );

		void OnLoaded();

		uint CountMeshes( aiNode* pNode );
		void LoadMeshes( aiNode* pNode );
		void LoadMesh( aiMesh* pMesh );

		aiScene* m_pScene;
	};

	struct LoadCommands
	{
		Array< TextureLoadCommand > m_aTextureLoadCommands;
		Array< ModelLoadCommand >	m_aModelLoadCommands;

		uint Count() const;
		bool Empty() const;
		void Grab( LoadCommands& oLoadCommands );
		void Clear();
	};

	using TextureResourceMap = std::unordered_map< std::filesystem::path, StrongPtr< TextureResource > >;
	using ModelResourceMap = std::unordered_map< std::filesystem::path, StrongPtr< ModelResource > >;

	TextureResourceMap		m_mTextureResources;
	ModelResourceMap		m_mModelResources;

	LoadCommands			m_oPendingLoadCommands;
	LoadCommands			m_oProcessingLoadCommands;

	std::mutex				m_oProcessingCommandsMutex;
	std::condition_variable m_oProcessingCommandsConditionVariable;
	std::thread				m_oIOThread;

	Assimp::Importer		m_oModelImporter;
};