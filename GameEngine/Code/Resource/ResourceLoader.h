#pragma once

#include <condition_variable>
#include <format>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <assimp/Importer.hpp>

#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Core/stb_truetype.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "ResourceTypes.h"

struct aiMesh;
struct aiNode;
struct aiScene;

struct FontLoadCommand;
struct TextureLoadCommand;
struct ModelLoadCommand;
struct ShaderLoadCommand;
struct TechniqueLoadCommand;
struct MaterialLoadCommand;

class ResourceLoader
{
public:
	template < typename LoadCommand >
	friend void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName );

	template < typename LoadCommand >
	friend uint CheckFinishedProcessingLoadCommands( Array< LoadCommand >& aLoadCommands );

	template < typename LoadCommand >
	friend uint CheckWaitingDependenciesLoadCommands( Array< LoadCommand >& aLoadCommands );

	friend class GameWorld;

	friend struct ModelLoadCommand;

	ResourceLoader();
	~ResourceLoader();

	FontResPtr		LoadFont( const char* sFilePath );
	TextureResPtr	LoadTexture( const char* sFilePath, const bool bSRGB = false, const bool bUse16Bits = false );
	TextureResPtr	LoadTexture( const char* sFilePath, const uint8* pData, const uint uDataSize, const bool bSRGB = false, const bool bUse16Bits = false );
	ModelResPtr		LoadModel( const char* sFilePath );
	ShaderResPtr	LoadShader( const char* sFilePath );
	TechniqueResPtr LoadTechnique( const char* sFilePath );
	MaterialResPtr	LoadMaterial( const char* sFilePath );

	void			HandleLoadedResources();
	void			ProcessLoadCommands();

	void			DisplayDebug();

private:
	void			Load();
	void			ProcessPendingLoadCommands();
	void			CheckFinishedProcessingLoadCommands();
	void			DestroyUnusedResources();

	struct LoadCommands
	{
		Array< FontLoadCommand >		m_aFontLoadCommands;
		Array< TextureLoadCommand >		m_aTextureLoadCommands;
		Array< ModelLoadCommand >		m_aModelLoadCommands;
		Array< ShaderLoadCommand >		m_aShaderLoadCommands;
		Array< TechniqueLoadCommand >	m_aTechniqueLoadCommands;
		Array< MaterialLoadCommand >	m_aMaterialLoadCommands;

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
	using MaterialResourceMap = std::unordered_map< std::string, MaterialResPtr >;

	FontResourceMap			m_mFontResources;
	TextureResourceMap		m_mTextureResources;
	ModelResourceMap		m_mModelResources;
	ShaderResourceMap		m_mShaderResources;
	TechniqueResourceMap	m_mTechniqueResources;
	MaterialResourceMap		m_mMaterialResources;

	LoadCommands			m_oPendingLoadCommands;
	LoadCommands			m_oProcessingLoadCommands;
	LoadCommands			m_oWaitingDependenciesLoadCommands;

	std::atomic_bool		m_bRunning;

	std::mutex				m_oProcessingCommandsMutex;
	std::condition_variable m_oProcessingCommandsConditionVariable;
	std::jthread			m_oIOThread;

	Assimp::Importer		m_oModelImporter;

	bool					m_bDisableUnusedResourcesDestruction;
	bool					m_bDisplayDebug;
};

extern ResourceLoader* g_pResourceLoader;