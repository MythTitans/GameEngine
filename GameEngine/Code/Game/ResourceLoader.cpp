#include "ResourceLoader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <nlohmann/json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "Core/Common.h"
#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Core/stb_image.h"
#include "Core/stb_truetype.h"
#include "DebugDisplay.h"

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

void TextureResource::Destroy()
{
	m_oTexture.Destroy();
}

Texture& TextureResource::GetTexture()
{
	return m_oTexture;
}

const Texture& TextureResource::GetTexture() const
{
	return m_oTexture;
}

void ModelResource::Destroy()
{
	for( Mesh& oMesh : m_aMeshes )
		oMesh.Destroy();
}

Array< Material >& ModelResource::GetMaterials()
{
	return m_aMaterials;
}

const Array< Material >& ModelResource::GetMaterials() const
{
	return m_aMaterials;
}

Array< Mesh >& ModelResource::GetMeshes()
{
	return m_aMeshes;
}

const Array< Mesh >& ModelResource::GetMeshes() const
{
	return m_aMeshes;
}

void ShaderResource::Destroy()
{
	m_oShader.Destroy();
}

Shader& ShaderResource::GetShader()
{
	return m_oShader;
}

const Shader& ShaderResource::GetShader() const
{
	return m_oShader;
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

ResourceLoader* g_pResourceLoader = nullptr;

//constexpr uint IO_THREAD_AFFINITY_MASK = 1 << 1;

ResourceLoader::ResourceLoader()
	: m_bRunning( true )
	, m_oIOThread( &ResourceLoader::Load, this )
{
	//SetThreadAffinityMask( m_oIOThread.native_handle(), IO_THREAD_AFFINITY_MASK );
	SetThreadDescription( m_oIOThread.native_handle(), L"IO thread" );

	g_pResourceLoader = this;
}

ResourceLoader::~ResourceLoader()
{
	m_bRunning = false;
	m_oProcessingCommandsConditionVariable.notify_one();

	g_pResourceLoader = nullptr;
}

FontResPtr ResourceLoader::LoadFont( const char* sFilePath )
{
	FontResPtr& xFontPtr = m_mFontResources[ sFilePath ];
	if( xFontPtr != nullptr )
		return xFontPtr;

	xFontPtr = new FontResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aFontLoadCommands.PushBack( FontLoadCommand( sFilePath, xFontPtr ) );

	return xFontPtr;
}

TextureResPtr ResourceLoader::LoadTexture( const char* sFilePath )
{
	TextureResPtr& xTexturePtr = m_mTextureResources[ sFilePath ];
	if( xTexturePtr != nullptr )
		return xTexturePtr;

	xTexturePtr = new TextureResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aTextureLoadCommands.PushBack( TextureLoadCommand( sFilePath, xTexturePtr ) );

	return xTexturePtr;
}

ModelResPtr ResourceLoader::LoadModel( const char* sFilePath )
{
	ModelResPtr& xModelPtr = m_mModelResources[ sFilePath ];
	if( xModelPtr != nullptr )
		return xModelPtr;

	xModelPtr = new ModelResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aModelLoadCommands.PushBack( ModelLoadCommand( sFilePath, xModelPtr ) );

	return xModelPtr;
}

ShaderResPtr ResourceLoader::LoadShader( const char* sFilePath )
{
	ShaderResPtr& xShaderPtr = m_mShaderResources[ sFilePath ];
	if( xShaderPtr != nullptr )
		return xShaderPtr;

	xShaderPtr = new ShaderResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aShaderLoadCommands.PushBack( ShaderLoadCommand( sFilePath, xShaderPtr ) );

	return xShaderPtr;
}

TechniqueResPtr ResourceLoader::LoadTechnique( const char* sFilePath )
{
	TechniqueResPtr& xTechniquePtr = m_mTechniqueResources[ sFilePath ];
	if( xTechniquePtr != nullptr )
		return xTechniquePtr;

	xTechniquePtr = new TechniqueResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aTechniqueLoadCommands.PushBack( TechniqueLoadCommand( sFilePath, xTechniquePtr ) );

	return xTechniquePtr;
}

void ResourceLoader::HandleLoadedResources()
{
	ProfilerBlock oBlock( "HandleLoadedResources" );

	CheckFinishedProcessingLoadCommands();
	DestroyUnusedResources();
}

void ResourceLoader::ProcessLoadCommands()
{
	ProfilerBlock oBlock( "ProcessLoadCommands" );

	ProcessPendingLoadCommands();
}

template < typename LoadCommand >
void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName )
{
	for( LoadCommand& oLoadCommand : aLoadCommands )
	{
		if( oLoadCommand.m_eStatus != ResourceLoader::LoadCommandStatus::PENDING )
			continue;

		ProfilerBlock oResourceBlock( sCommandName, true );

		{
			ProfilerBlock oResourceBlock( "CheckResource", true );

			if( std::filesystem::exists( oLoadCommand.GetFilePath() ) == false )
			{
				oLock.lock();
				oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::NOT_FOUND;
				oLock.unlock();
				continue;
			}
		}

		oLoadCommand.Load( oLock );
	}
}

void ResourceLoader::Load()
{
	while( m_bRunning )
	{
		std::unique_lock oLock( m_oProcessingCommandsMutex );
		// TODO #eric improve load queue handling
		m_oProcessingCommandsConditionVariable.wait( oLock, [ this ]() { return m_oProcessingLoadCommands.Empty() == false || m_bRunning == false; } );
		oLock.unlock();

		if( m_bRunning == false )
			return;

		::Load( m_oProcessingLoadCommands.m_aFontLoadCommands, oLock, "LoadFont" );
		::Load( m_oProcessingLoadCommands.m_aShaderLoadCommands, oLock, "LoadShader" );
		::Load( m_oProcessingLoadCommands.m_aTechniqueLoadCommands, oLock, "LoadTechnique" );
		::Load( m_oProcessingLoadCommands.m_aTextureLoadCommands, oLock, "LoadTexture" );
		::Load( m_oProcessingLoadCommands.m_aModelLoadCommands, oLock, "LoadModel" );
	}
}

void ResourceLoader::ProcessPendingLoadCommands()
{
	ProfilerBlock oBlock( "ProcessLoadCommands" );

	std::unique_lock oLock( m_oProcessingCommandsMutex );
	if( m_oProcessingLoadCommands.Empty() && m_oPendingLoadCommands.Empty() == false )
	{
		m_oProcessingLoadCommands.Grab( m_oPendingLoadCommands );
		oLock.unlock();

		m_oProcessingCommandsConditionVariable.notify_one();
	}
}

template < typename LoadCommand >
uint CheckFinishedProcessingLoadCommands( Array< LoadCommand >& aLoadCommands )
{
	uint uFinishedCount = 0;

	for( uint u = 0; u < aLoadCommands.Count(); ++u )
	{
		LoadCommand& oLoadCommand = aLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case ResourceLoader::LoadCommandStatus::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::NOT_FOUND;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case ResourceLoader::LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::ERROR_READING;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case ResourceLoader::LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			oLoadCommand.OnFinished();
			if( oLoadCommand.HasDependencies() )
			{
				LOG_INFO( "Waiting dependencies for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::WAITING_DEPENDENCIES;
			}
			++uFinishedCount;
			break;
		case ResourceLoader::LoadCommandStatus::FINISHED:
		case ResourceLoader::LoadCommandStatus::WAITING_DEPENDENCIES:
			++uFinishedCount;
			break;
		}
	}

	return uFinishedCount;
}

template < typename LoadCommand >
uint CheckWaitingDependenciesLoadCommands( Array< LoadCommand >& aLoadCommands )
{
	uint uFinishedCount = 0;

	for( uint u = 0; u < aLoadCommands.Count(); ++u )
	{
		LoadCommand& oLoadCommand = aLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case ResourceLoader::LoadCommandStatus::WAITING_DEPENDENCIES:
			if( oLoadCommand.AllDependenciesLoaded() )
			{
				LOG_INFO( "Dependencies ready for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
				oLoadCommand.OnDependenciesReady();
				++uFinishedCount;
			}
			else if( oLoadCommand.AnyDependencyFailed() )
			{
				LOG_ERROR( "Failed to load a dependency for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::ERROR_READING;
				oLoadCommand.OnDependenciesReady();
				++uFinishedCount;
			}
			break;
		case ResourceLoader::LoadCommandStatus::FINISHED:
			++uFinishedCount;
			break;
		default:
			break;
		}
	}

	return uFinishedCount;
}

void ResourceLoader::CheckFinishedProcessingLoadCommands()
{
	ProfilerBlock oBlock( "CheckFinishedLoadCommands" );

	uint uFinishedCount = 0;

	std::unique_lock oLock( m_oProcessingCommandsMutex );

	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aFontLoadCommands );
	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aShaderLoadCommands );
	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aTechniqueLoadCommands );
	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aTextureLoadCommands );
	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aModelLoadCommands );

	if( uFinishedCount == m_oProcessingLoadCommands.Count() )
	{
		m_oWaitingDependenciesLoadCommands.CopyWaitingDependencies( m_oProcessingLoadCommands );
		m_oProcessingLoadCommands.Clear();
	}

	uFinishedCount = 0;

	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aFontLoadCommands );
	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aShaderLoadCommands );
	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aTechniqueLoadCommands );
	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aTextureLoadCommands );
	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aModelLoadCommands );

	// TODO #eric not ideal, the list will only be cleared when all waiting commands are completed at the same time
	if( uFinishedCount == m_oWaitingDependenciesLoadCommands.Count() )
		m_oWaitingDependenciesLoadCommands.Clear();

	if( m_oPendingLoadCommands.Empty() == false )
		g_pDebugDisplay->DisplayText( std::format( "Pending load commands {}", m_oPendingLoadCommands.Count() ), glm::vec4( 1.f, 0.5f, 0.f, 1.f ) );

	if( m_oProcessingLoadCommands.Empty() == false )
		g_pDebugDisplay->DisplayText( std::format( "Processing load commands {}", m_oProcessingLoadCommands.Count() ), glm::vec4( 0.f, 0.5f, 1.f, 1.f ) );

	if( m_oWaitingDependenciesLoadCommands.Empty() == false )
		g_pDebugDisplay->DisplayText( std::format( "Waiting dependencies load commands {}", m_oWaitingDependenciesLoadCommands.Count() ), glm::vec4( 0.5f, 1.f, 0.f, 1.f ) );
}

template < typename Resource >
void DestroyUnusedResources( std::unordered_map< std::string, StrongPtr< Resource > >& mResources )
{
	for( auto& oPair : mResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			LOG_INFO( "Unloading {}", oPair.first );
			oPair.second->Destroy();
			oPair.second = nullptr;
		}
	}

	std::erase_if( mResources, []( const std::pair< std::string, StrongPtr< Resource > >& oPair ) { return oPair.second == nullptr; } );
}

void ResourceLoader::DestroyUnusedResources()
{
	ProfilerBlock oBlock( "DestroyUnusedResources" );

	::DestroyUnusedResources( m_mFontResources );
	::DestroyUnusedResources( m_mTechniqueResources );
	::DestroyUnusedResources( m_mShaderResources );
	::DestroyUnusedResources( m_mTextureResources );
	::DestroyUnusedResources( m_mModelResources );
}

ResourceLoader::FontLoadCommand::FontLoadCommand( const char* sFilePath, const FontResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
{
}

void ResourceLoader::FontLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	Array< uint8 > aAtlasData( FontResource::ATLAS_WIDTH * FontResource::ATLAS_HEIGHT );
	Array< stbtt_packedchar > aPackedCharacters( FontResource::GLYPH_COUNT );

	Array< uint8 > aFontData = ReadBinaryFile( GetFilePath() );

	stbtt_pack_context oAtlasContext;
	stbtt_PackBegin( &oAtlasContext, aAtlasData.Data(), FontResource::ATLAS_WIDTH, FontResource::ATLAS_HEIGHT, 0, 1, nullptr );
	stbtt_PackFontRange( &oAtlasContext, aFontData.Data(), 0, ( float )FontResource::FONT_HEIGHT, FontResource::FIRST_GLYPH, FontResource::GLYPH_COUNT, aPackedCharacters.Data() );
	stbtt_PackEnd( &oAtlasContext );

	oLock.lock();
	m_eStatus = aAtlasData.Empty() == false ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_aAtlasData = std::move( aAtlasData );
	m_aPackedCharacters = std::move( aPackedCharacters );
	oLock.unlock();
}

void ResourceLoader::FontLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
		m_xResource->m_oAtlas.Create( FontResource::ATLAS_WIDTH, FontResource::ATLAS_HEIGHT, TextureFormat::RED, m_aAtlasData.Data() );
		m_xResource->m_aPackedCharacters = std::move( m_aPackedCharacters );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ResourceLoader::FontLoadCommand::OnDependenciesReady()
{
}

ResourceLoader::TextureLoadCommand::TextureLoadCommand( const char* sFilePath, const TextureResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_iDepth( 0 )
	, m_pData( nullptr )
{
}

void ResourceLoader::TextureLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	int iWidth;
	int iHeight;
	int iDepth;
	uint8* pData = stbi_load( GetFilePath().string().c_str(), &iWidth, &iHeight, &iDepth, 0 );

	oLock.lock();
	m_eStatus = pData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;
	m_pData = pData;
	oLock.unlock();
}

void ResourceLoader::TextureLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
		m_xResource->m_oTexture.Create( m_iWidth, m_iHeight, m_iDepth == 3 ? TextureFormat::RGB : TextureFormat::RGBA, m_pData );
		stbi_image_free( m_pData );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ResourceLoader::TextureLoadCommand::OnDependenciesReady()
{
}

ResourceLoader::ModelLoadCommand::ModelLoadCommand( const char* sFilePath, const ModelResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
	, m_pScene( nullptr )
{
}

void ResourceLoader::ModelLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	aiScene* pSceneData = nullptr;

	const aiScene* pScene = g_pResourceLoader->m_oModelImporter.ReadFile( GetFilePath().string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace );
	if( pScene != nullptr )
		pSceneData = g_pResourceLoader->m_oModelImporter.GetOrphanedScene();

	oLock.lock();
	m_eStatus = pSceneData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	m_pScene = pSceneData;
	oLock.unlock();
}

void ResourceLoader::ModelLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
	{
		aiNode* pRoot = m_pScene->mRootNode;
		m_xResource->m_aMeshes.Reserve( CountMeshes( pRoot ) );
		LoadMaterials( m_pScene );
		LoadMeshes( pRoot );
		delete m_pScene;
		if( HasDependencies() == false )
			m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	}
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ResourceLoader::ModelLoadCommand::OnDependenciesReady()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}

	m_aDependencies.Clear();
}

uint ResourceLoader::ModelLoadCommand::CountMeshes( aiNode* pNode )
{
	uint uCount = pNode->mNumMeshes;

	for( uint u = 0; u < pNode->mNumChildren; ++u )
		uCount += CountMeshes( pNode->mChildren[ u ] );

	return uCount;
}

void ResourceLoader::ModelLoadCommand::LoadMaterials( aiScene* pScene )
{
	m_xResource->m_aMaterials.Resize( pScene->mNumMaterials );

	for( uint u = 0; u < pScene->mNumMaterials; ++u )
	{
		//TechniqueResPtr xTechniqueResource = g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/basic" ) );
		//m_xResource->m_aMaterials[ u ].m_xTechniqueResource = xTechniqueResource;
		//m_aDependencies.PushBack( xTechniqueResource.GetPtr() );

		const aiMaterial* pMaterial = pScene->mMaterials[ u ];

		aiColor3D oDiffuseColor;
		pMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, oDiffuseColor );
		m_xResource->m_aMaterials[ u ].m_vDiffuseColor = glm::vec3( oDiffuseColor.r, oDiffuseColor.g, oDiffuseColor.b );

		if( pMaterial->GetTextureCount( aiTextureType_DIFFUSE ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = g_pResourceLoader->LoadTexture( sFile.C_Str() );
				m_xResource->m_aMaterials[ u ].m_xDiffuseTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_NORMALS ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_NORMALS, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = g_pResourceLoader->LoadTexture( sFile.C_Str() );
				m_xResource->m_aMaterials[ u ].m_xNormalTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}
	}
}

void ResourceLoader::ModelLoadCommand::LoadMeshes( aiNode* pNode )
{
	for( uint u = 0; u < pNode->mNumMeshes; ++u )
		LoadMesh( m_pScene->mMeshes[ pNode->mMeshes[ u ] ] );

	for( uint u = 0; u < pNode->mNumChildren; ++u )
		LoadMeshes( pNode->mChildren[ u ] );
}

void ResourceLoader::ModelLoadCommand::LoadMesh( aiMesh* pMesh )
{
	const uint uVertexCount = pMesh->mNumVertices;

	Array< glm::vec3 > aVertices( uVertexCount );
	Array< glm::vec3 > aNormals( uVertexCount );
	Array< glm::vec3 > aTangents( uVertexCount );
	Array< glm::vec2 > aUVs( uVertexCount );

	for( uint u = 0; u < uVertexCount; ++u )
	{
		aVertices[ u ] = glm::vec3( pMesh->mVertices[ u ].x, pMesh->mVertices[ u ].y, pMesh->mVertices[ u ].z );
		aNormals[ u ] = glm::vec3( pMesh->mNormals[ u ].x, pMesh->mNormals[ u ].y, pMesh->mNormals[ u ].z );
		aTangents[ u ] = glm::vec3( pMesh->mTangents[ u ].x, pMesh->mTangents[ u ].y, pMesh->mTangents[ u ].z );

		if( pMesh->mTextureCoords[ 0 ] != nullptr )
			aUVs[ u ] = glm::vec2( pMesh->mTextureCoords[ 0 ][ u ].x, pMesh->mTextureCoords[ 0 ][ u ].y );
	}

	uint uIndexCount = 0;
	for( uint u = 0; u < pMesh->mNumFaces; ++u )
		uIndexCount += pMesh->mFaces[ u ].mNumIndices;

	Array< GLuint > aIndices;
	aIndices.Reserve( uIndexCount );

	for( uint uFace = 0; uFace < pMesh->mNumFaces; ++uFace )
	{
		const aiFace& oFace = pMesh->mFaces[ uFace ];
		for( uint uIndex = 0; uIndex < oFace.mNumIndices; ++uIndex )
			aIndices.PushBack( oFace.mIndices[ uIndex ] );
	}

	MeshBuilder oMeshBuilder = MeshBuilder( std::move( aVertices ), std::move( aIndices ) )
		.WithUVs( std::move( aUVs ) )
		.WithNormals( std::move( aNormals ) )
		.WithTangents( std::move( aTangents ) );

	if( pMesh->mMaterialIndex >= 0 && pMesh->mMaterialIndex < m_xResource->m_aMaterials.Count() )
		oMeshBuilder.WithMaterial( &m_xResource->m_aMaterials[ pMesh->mMaterialIndex ] );

	m_xResource->m_aMeshes.PushBack( oMeshBuilder.Build() );
}

ResourceLoader::ShaderLoadCommand::ShaderLoadCommand( const char* sFilePath, const ShaderResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
	, m_eShaderType( ShaderType::UNDEFINED )
{
	const std::filesystem::path oFilePath = GetFilePath();
	if( oFilePath.extension() == ".vs" )
		m_eShaderType = ShaderType::VERTEX_SHADER;
	else if( oFilePath.extension() == ".ps" )
		m_eShaderType = ShaderType::PIXEL_SHADER;
}

void ResourceLoader::ShaderLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( GetFilePath() );

	oLock.lock();
	m_eStatus = LoadCommandStatus::LOADED;
	m_sShaderCode = std::move( sContent );
	oLock.unlock();
}

void ResourceLoader::ShaderLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
		m_xResource->m_oShader.Create( m_sShaderCode, m_eShaderType );
		m_xResource->m_eStatus = Resource::Status::LOADED;
		break;
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ResourceLoader::ShaderLoadCommand::OnDependenciesReady()
{
}

ResourceLoader::TechniqueLoadCommand::TechniqueLoadCommand( const char* sFilePath, const TechniqueResPtr& xResource )
	: LoadCommand( sFilePath, xResource )
{
}

void ResourceLoader::TechniqueLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( GetFilePath() );

	std::string sVertexShader;
	std::string sPixelShader;
	Array< std::string > aParameters;
	Array< std::pair< std::string, uint > > aArrayParameters;

	bool bSuccess = true;

	try
	{
		const nlohmann::json oJsonContent = nlohmann::json::parse( sContent );

		sVertexShader = oJsonContent.at( "vertexShader" );
		sPixelShader = oJsonContent.at( "pixelShader" );

		if( oJsonContent.contains( "parameters" ) )
		{
			const nlohmann::json& oParameters = oJsonContent[ "parameters" ];
			aParameters.Reserve( ( uint )oParameters.size() );
			for( const std::string& sParameter : oParameters )
				aParameters.PushBack( sParameter );
		}

		if( oJsonContent.contains( "arrays" ) )
		{
			const nlohmann::json& oArrays = oJsonContent[ "arrays" ];
			aArrayParameters.Reserve( ( uint )oArrays.size() );
			for( const nlohmann::json& oArray : oArrays )
			{
				const uint uCount = oArray.at( "count" );
				if( oArray.contains( "parameter" ) )
					aArrayParameters.PushBack( std::pair( oArray[ "parameter" ], uCount ) );

				if( oArray.contains( "parameters" ) )
				{
					aArrayParameters.Reserve( aArrayParameters.Count() + ( uint )oArray[ "parameters" ].size() );
					for( const std::string& sParameter : oArray[ "parameters" ] )
						aArrayParameters.PushBack( std::pair( sParameter, uCount ) );
				}
			}
		}
	}
	catch( const std::exception& oException )
	{
		LOG_ERROR( "{} : {}", GetFilePath().string(), oException.what() );
		bSuccess = false;
	}

	oLock.lock();
	if( bSuccess )
	{
		m_eStatus = LoadCommandStatus::LOADED;
		m_aParameters = std::move( aParameters );
		m_aArrayParameters = std::move( aArrayParameters );
		m_aDependencies.PushBack( g_pResourceLoader->LoadShader( sVertexShader.c_str() ).GetPtr() );
		m_aDependencies.PushBack( g_pResourceLoader->LoadShader( sPixelShader.c_str() ).GetPtr() );
	}
	else
	{
		m_eStatus = LoadCommandStatus::ERROR_READING;
	}
	oLock.unlock();
}

void ResourceLoader::TechniqueLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}
}

void ResourceLoader::TechniqueLoadCommand::OnDependenciesReady()
{
	switch( m_eStatus )
	{
	case LoadCommandStatus::FINISHED:
	{
		Array< const Shader* > aShaders;
		aShaders.Reserve( m_aDependencies.Count() );
		for( const StrongPtr< Resource >& pDependency : m_aDependencies )
		{
			const ShaderResource* pShaderResource = static_cast< const ShaderResource* >( pDependency.GetPtr() );
			ASSERT( pShaderResource->IsLoaded() );
			aShaders.PushBack( &pShaderResource->m_oShader );
		}
		m_xResource->m_oTechnique.Create( aShaders, m_aParameters, m_aArrayParameters );

		m_xResource->m_eStatus = Resource::Status::LOADED;

		break;
	}
	case ResourceLoader::LoadCommandStatus::NOT_FOUND:
	case ResourceLoader::LoadCommandStatus::ERROR_READING:
		m_xResource->m_eStatus = Resource::Status::FAILED;
		break;
	default:
		break;
	}

	m_xResource->m_aShaderResources = std::move( m_aDependencies );
}

uint ResourceLoader::LoadCommands::Count() const
{
	return m_aFontLoadCommands.Count() + m_aTextureLoadCommands.Count() + m_aModelLoadCommands.Count() + m_aShaderLoadCommands.Count() + m_aTechniqueLoadCommands.Count();
}

bool ResourceLoader::LoadCommands::Empty() const
{
	return m_aFontLoadCommands.Empty() && m_aTextureLoadCommands.Empty() && m_aModelLoadCommands.Empty() && m_aShaderLoadCommands.Empty() && m_aTechniqueLoadCommands.Empty();
}

void ResourceLoader::LoadCommands::Grab( LoadCommands& oLoadCommands )
{
	m_aFontLoadCommands.Grab( oLoadCommands.m_aFontLoadCommands );
	m_aTextureLoadCommands.Grab( oLoadCommands.m_aTextureLoadCommands );
	m_aModelLoadCommands.Grab( oLoadCommands.m_aModelLoadCommands );
	m_aShaderLoadCommands.Grab( oLoadCommands.m_aShaderLoadCommands );
	m_aTechniqueLoadCommands.Grab( oLoadCommands.m_aTechniqueLoadCommands );
}

void ResourceLoader::LoadCommands::CopyWaitingDependencies( LoadCommands& oLoadCommands )
{
	m_aFontLoadCommands.Expand( oLoadCommands.m_aFontLoadCommands.Count() );
	m_aTextureLoadCommands.Expand( oLoadCommands.m_aTextureLoadCommands.Count() );
	m_aModelLoadCommands.Expand( oLoadCommands.m_aModelLoadCommands.Count() );
	m_aShaderLoadCommands.Expand( oLoadCommands.m_aShaderLoadCommands.Count() );
	m_aTechniqueLoadCommands.Expand( oLoadCommands.m_aTechniqueLoadCommands.Count() );

	for( const FontLoadCommand& oLoadCommand : oLoadCommands.m_aFontLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aFontLoadCommands.PushBack( oLoadCommand );
	}

	for( const TextureLoadCommand& oLoadCommand : oLoadCommands.m_aTextureLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aTextureLoadCommands.PushBack( oLoadCommand );
	}

	for( const ModelLoadCommand& oLoadCommand : oLoadCommands.m_aModelLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aModelLoadCommands.PushBack( oLoadCommand );
	}

	for( const ShaderLoadCommand& oLoadCommand : oLoadCommands.m_aShaderLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aShaderLoadCommands.PushBack( oLoadCommand );
	}

	for( const TechniqueLoadCommand& oLoadCommand : oLoadCommands.m_aTechniqueLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aTechniqueLoadCommands.PushBack( oLoadCommand );
	}
}

void ResourceLoader::LoadCommands::Clear()
{
	m_aFontLoadCommands.Clear();
	m_aTextureLoadCommands.Clear();
	m_aModelLoadCommands.Clear();
	m_aShaderLoadCommands.Clear();
	m_aTechniqueLoadCommands.Clear();
}
