#pragma once

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Core/stb_truetype.h"
#include "Game/Animation.h"
#include "Graphics/Material.h"
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

class ShaderResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

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

class TextureResource : public Resource
{
public:
	friend class ResourceLoader;

	void			Destroy() override;

	const Texture&	GetTexture() const;

private:
	Texture m_oTexture;
};

using TextureResPtr = StrongPtr< TextureResource >;

class ModelResource : public Resource
{
public:
	friend class ResourceLoader;

	void						Destroy() override;

	const Array< Mesh >&		GetMeshes() const;

	const Array< Animation >&	GetAnimations() const;
	const Skeleton&				GetSkeleton() const;

	const Array< glm::mat4 >&	GetPoseMatrices() const;
	const Array< glm::mat4 >&	GetSkinMatrices() const;

private:
	Array< Mesh >			m_aMeshes;
	Array< Animation >		m_aAnimations;
	Skeleton				m_oSkeleton;
	Array < glm::mat4 >		m_aPoseMatrices;
	Array < glm::mat4 >		m_aSkinMatrices;
};

using ModelResPtr = StrongPtr< ModelResource >;

class ResourceLoader
{
public:
	template < typename LoadCommand >
	friend void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName );

	template < typename LoadCommand >
	friend uint CheckFinishedProcessingLoadCommands( Array< LoadCommand >& aLoadCommands );

	template < typename LoadCommand >
	friend uint CheckWaitingDependenciesLoadCommands( Array< LoadCommand >& aLoadCommands );

	ResourceLoader();
	~ResourceLoader();

	FontResPtr		LoadFont( const char* sFilePath );
	TextureResPtr	LoadTexture( const char* sFilePath, const bool bSRGB = false );
	TextureResPtr	LoadTexture( const char* sFilePath, const uint8* pData, const uint uDataSize, const bool bSRGB = false );
	ModelResPtr		LoadModel( const char* sFilePath );
	ShaderResPtr	LoadShader( const char* sFilePath );
	TechniqueResPtr LoadTechnique( const char* sFilePath );

	void			HandleLoadedResources();
	void			ProcessLoadCommands();

	void			DisplayDebug();

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
		{
		}

		std::filesystem::path GetFilePath() const
		{
			return std::filesystem::path( std::format( "Data/{}", m_sFilePath ) );
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
		TextureLoadCommand( const char* sFilePath, const TextureResPtr& xResource, const bool bSRGB );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;
		void OnDependenciesReady() override;

		int		m_iWidth;
		int		m_iHeight;
		int		m_iDepth;
		bool	m_bSRGB;
		uint8*	m_pData;
	};

	struct ModelLoadCommand : LoadCommand< ModelResource >
	{
		ModelLoadCommand( const char* sFilePath, const ModelResPtr& xResource );

		void						Load( std::unique_lock< std::mutex >& oLock ) override;
		void						OnFinished() override;
		void						OnDependenciesReady() override;

		void						LoadAnimations();
		void						LoadSkeleton();
		void						LoadMaterials();
		void						LoadMeshes();
		uint						CountMeshes( aiNode* pNode );
		void						LoadMeshes( aiNode* pNode );
		void						LoadMesh( aiMesh* pMesh );
		void						LoadSkeleton( aiNode* pNode, Skeleton& oParent );
		TextureResPtr				LoadTexture( const std::string& sFileName, const bool bSRGB = false );
		uint						FetchNodeIndex( const std::string& sName );

		aiScene*								m_pScene;
		Array< LitMaterialData >				m_aMaterials;
		std::unordered_map< std::string, uint > m_mNodeIndices;
	};

	struct ShaderLoadCommand : LoadCommand< ShaderResource >
	{
		ShaderLoadCommand( const char* sFilePath, const ShaderResPtr& xResource );

		void Load( std::unique_lock< std::mutex >& oLock ) override;
		void OnFinished() override;
		void OnDependenciesReady() override;

		std::string m_sShaderCode;
		ShaderType	m_eShaderType;
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
		void CopyWaitingDependencies( LoadCommands& oLoadCommands );
		void Clear();
	};

	using FontResourceMap = std::unordered_map< std::string, FontResPtr >;
	using TextureResourceMap = std::unordered_map< std::string, TextureResPtr >;
	using ModelResourceMap = std::unordered_map< std::string, ModelResPtr >;
	using ShaderResourceMap = std::unordered_map< std::string, ShaderResPtr >;
	using TechniqueResourceMap = std::unordered_map< std::string, TechniqueResPtr >;

	FontResourceMap			m_mFontResources;
	TextureResourceMap		m_mTextureResources;
	ModelResourceMap		m_mModelResources;
	ShaderResourceMap		m_mShaderResources;
	TechniqueResourceMap	m_mTechniqueResources;

	LoadCommands			m_oPendingLoadCommands;
	LoadCommands			m_oProcessingLoadCommands;
	LoadCommands			m_oWaitingDependenciesLoadCommands;

	std::atomic_bool		m_bRunning;

	std::mutex				m_oProcessingCommandsMutex;
	std::condition_variable m_oProcessingCommandsConditionVariable;
	std::jthread			m_oIOThread;

	Assimp::Importer		m_oModelImporter;

	bool					m_bDisplayDebug;
};

extern ResourceLoader* g_pResourceLoader;