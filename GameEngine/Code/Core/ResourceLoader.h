#pragma once

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <assimp/Importer.hpp>

#include "stb_truetype.h"

#include "Array.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"
#include "Graphics/Technique.h"
#include "Graphics/Texture.h"
#include "Intrusive.h"

struct aiMesh;
struct aiNode;
struct aiScene;

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

using FontResPtr = StrongPtr< FontResource >;

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
	Technique						m_oTechnique;

	Array< StrongPtr< Resource > >	m_aShaderResources;
};

using TechniqueResPtr = StrongPtr< TechniqueResource >;

class ResourceLoader
{
public:
	template < typename LoadCommand >
	friend void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName, const bool bIgnoreFileCheck );

	template < typename LoadCommand >
	friend uint CheckFinishedProcessingLoadCommands( Array< LoadCommand >& aLoadCommands );

	ResourceLoader();
	~ResourceLoader();

	FontResPtr		LoadFont( const std::filesystem::path& oFilePath );
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

	template < typename Res >
	struct LoadCommand
	{
		LoadCommand( const std::filesystem::path& oFilePath, const StrongPtr< Res >& xResource )
			: m_oFilePath( oFilePath )
			, m_xResource( xResource )
			, m_eStatus( LoadCommandStatus::PENDING )
		{
		}

		virtual ~LoadCommand()
		{
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

		std::filesystem::path			m_oFilePath;
		StrongPtr< Res >				m_xResource;
		LoadCommandStatus				m_eStatus;
		Array< StrongPtr< Resource > >	m_aDependencies;
	};

	struct FontLoadCommand : LoadCommand< FontResource >
	{
		FontLoadCommand( const std::filesystem::path& oFilePath, const FontResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;

		Array< uint8 >				m_aAtlasData;
		Array< stbtt_packedchar >	m_aPackedCharacters;
	};

	struct TextureLoadCommand : LoadCommand< TextureResource >
	{
		TextureLoadCommand( const std::filesystem::path& oFilePath, const TextureResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;

		int		m_iWidth;
		int		m_iHeight;
		int		m_iDepth;
		uint8*	m_pData;
	};

	struct ModelLoadCommand : LoadCommand< ModelResource >
	{
		ModelLoadCommand( const std::filesystem::path& oFilePath, const ModelResPtr& xResource, Assimp::Importer& oModelImporter );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;

		uint CountMeshes( aiNode* pNode );
		void LoadMeshes( aiNode* pNode );
		void LoadMesh( aiMesh* pMesh );

		Assimp::Importer&	m_oModelImporter;
		aiScene*			m_pScene;
	};

	struct ShaderLoadCommand : LoadCommand< ShaderResource >
	{
		ShaderLoadCommand( const std::filesystem::path& oFilePath, const ShaderResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;

		std::string m_sShaderCode;
		ShaderType	m_eShaderType;
	};

	struct TechniqueLoadCommand : LoadCommand< TechniqueResource >
	{
		TechniqueLoadCommand( const std::filesystem::path& oFilePath, const TechniqueResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;
	};

	struct LoadCommands
	{
		Array< FontLoadCommand >		m_aFontLoadCommands;
		Array< TextureLoadCommand >		m_aTextureLoadCommands;
		Array< ModelLoadCommand >		m_aModelLoadCommands;
		Array< ShaderLoadCommand >		m_aShaderLoadCommands;
		Array< TechniqueLoadCommand >	m_aTechniqueLoadCommands;

		uint Count() const;
		bool Empty() const;
		void Grab( LoadCommands& oLoadCommands );
		void Clear();
	};

	using FontResourceMap = std::unordered_map< std::filesystem::path, FontResPtr >;
	using TextureResourceMap = std::unordered_map< std::filesystem::path, TextureResPtr >;
	using ModelResourceMap = std::unordered_map< std::filesystem::path, ModelResPtr >;
	using ShaderResourceMap = std::unordered_map< std::filesystem::path, ShaderResPtr >;
	using TechniqueResourceMap = std::unordered_map< std::filesystem::path, TechniqueResPtr >;

	FontResourceMap			m_mFontResources;
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