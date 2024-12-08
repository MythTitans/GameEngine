#include "ResourceLoader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "Common.h"
#include "Game/GameEngine.h"
#include "FileUtils.h"
#include "Graphics/Utils.h"
#include "Logger.h"
#include "Profiler.h"

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

FontResPtr ResourceLoader::LoadFont( const std::filesystem::path& oFilePath )
{
	FontResPtr& xFontPtr = m_mFontResources[ oFilePath ];
	if( xFontPtr != nullptr )
		return xFontPtr;

	xFontPtr = new FontResource();

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_oPendingLoadCommands.m_aFontLoadCommands.PushBack( FontLoadCommand( oFilePath, xFontPtr ) );

	return xFontPtr;
}

TextureResPtr ResourceLoader::LoadTexture( const std::filesystem::path& oFilePath )
{
	TextureResPtr& xTexturePtr = m_mTextureResources[ oFilePath ];
	if( xTexturePtr != nullptr )
		return xTexturePtr;

	xTexturePtr = new TextureResource();

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_oPendingLoadCommands.m_aTextureLoadCommands.PushBack( TextureLoadCommand( oFilePath, xTexturePtr ) );

	return xTexturePtr;
}

ModelResPtr ResourceLoader::LoadModel( const std::filesystem::path& oFilePath )
{
	ModelResPtr& xModelPtr = m_mModelResources[ oFilePath ];
	if( xModelPtr != nullptr )
		return xModelPtr;

	xModelPtr = new ModelResource();

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_oPendingLoadCommands.m_aModelLoadCommands.PushBack( ModelLoadCommand( oFilePath, xModelPtr ) );

	return xModelPtr;
}

ShaderResPtr ResourceLoader::LoadShader( const std::filesystem::path& oFilePath )
{
	ShaderResPtr& xShaderPtr = m_mShaderResources[ oFilePath ];
	if( xShaderPtr != nullptr )
		return xShaderPtr;

	xShaderPtr = new ShaderResource();

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_oPendingLoadCommands.m_aShaderLoadCommands.PushBack( ShaderLoadCommand( oFilePath, xShaderPtr ) );

	return xShaderPtr;
}

TechniqueResPtr ResourceLoader::LoadTechnique( const std::filesystem::path& oFilePath )
{
	TechniqueResPtr& xTechniquePtr = m_mTechniqueResources[ oFilePath ];
	if( xTechniquePtr != nullptr )
		return xTechniquePtr;

	xTechniquePtr = new TechniqueResource();

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_oPendingLoadCommands.m_aTechniqueLoadCommands.PushBack( TechniqueLoadCommand( oFilePath, xTechniquePtr ) );

	return xTechniquePtr;
}

void ResourceLoader::HandleLoadedResources()
{
	ProfilerBlock oBlock( "HandleLoadedResources" );

	CheckFinishedProcessingLoadCommands();
	DestroyUnusedResources();
}

void ResourceLoader::Update()
{
	ProfilerBlock oBlock( "ResourceLoader" );

	LoadTexture( std::filesystem::path( "Data/WHITE.png" ) );
	LoadTexture( std::filesystem::path( "Data/BLACK.png" ) );
	LoadTexture( std::filesystem::path( "Data/RED.png" ) );
	LoadTexture( std::filesystem::path( "Data/GREEN.png" ) );
	LoadTexture( std::filesystem::path( "Data/BLUE.png" ) );
	LoadTexture( std::filesystem::path( "Data/TRANSPARENT.png" ) );
	LoadModel( std::filesystem::path( "Data/cube.obj" ) );
	LoadTechnique( std::filesystem::path( "Data/basic" ) );
	//LoadShader( std::filesystem::path( "Data/basic.vs" ) );
	//LoadShader( std::filesystem::path( "Data/basic.ps" ) );
	LoadFont( std::filesystem::path( "C:/Windows/Fonts/arialbd.ttf" ) );
}

void ResourceLoader::ProcessLoadCommands()
{
	ProfilerBlock oBlock( "ProcessLoadCommands" );

	ProcessPendingLoadCommands();
}

template < typename LoadCommand >
void Load( Array< LoadCommand >& aLoadCommands, std::unique_lock< std::mutex >& oLock, const char* sCommandName, const bool bIgnoreFileCheck = false )
{
	for( LoadCommand& oLoadCommand : aLoadCommands )
	{
		if( oLoadCommand.m_eStatus != ResourceLoader::LoadCommandStatus::PENDING )
			continue;

		ProfilerBlock oResourceBlock( sCommandName, true );

		{
			ProfilerBlock oResourceBlock( "CheckResource", true );

			if( bIgnoreFileCheck == false && std::filesystem::exists( oLoadCommand.m_oFilePath ) == false )
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
		::Load( m_oProcessingLoadCommands.m_aTechniqueLoadCommands, oLock, "LoadTechnique", true );
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
			LOG_ERROR( "File not found {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::NOT_FOUND;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case ResourceLoader::LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::ERROR_READING;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case ResourceLoader::LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
			oLoadCommand.OnFinished();
			if( oLoadCommand.HasDependencies() )
			{
				LOG_INFO( "Waiting dependencies for {}", oLoadCommand.m_oFilePath.string() );
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
				LOG_INFO( "Dependencies ready for {}", oLoadCommand.m_oFilePath.string() );
				oLoadCommand.m_eStatus = ResourceLoader::LoadCommandStatus::FINISHED;
				oLoadCommand.OnDependenciesReady();
				++uFinishedCount;
			}
			else if( oLoadCommand.AnyDependencyFailed() )
			{
				LOG_INFO( "Failed to load a dependency for {}", oLoadCommand.m_oFilePath.string() );
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
		g_pGameEngine->GetDebugDisplay().DisplayText( std::format( "Pending load commands {}", m_oPendingLoadCommands.Count() ), glm::vec4( 1.f, 0.5f, 0.f, 1.f ) );

	if( m_oProcessingLoadCommands.Empty() == false )
		g_pGameEngine->GetDebugDisplay().DisplayText( std::format( "Processing load commands {}", m_oProcessingLoadCommands.Count() ), glm::vec4( 0.f, 0.5f, 1.f, 1.f ) );

	if( m_oWaitingDependenciesLoadCommands.Empty() == false )
		g_pGameEngine->GetDebugDisplay().DisplayText( std::format( "Waiting dependencies load commands {}", m_oWaitingDependenciesLoadCommands.Count() ), glm::vec4( 0.5f, 1.f, 0.f, 1.f ) );
}

template < typename Resource >
void DestroyUnusedResources( std::unordered_map< std::filesystem::path, StrongPtr< Resource > >& mResources )
{
	for( auto& oPair : mResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			LOG_INFO( "Unloading {}", oPair.first.string() );
			oPair.second->Destroy();
			oPair.second = nullptr;
		}
	}

	std::erase_if( mResources, []( const std::pair< std::filesystem::path, StrongPtr< Resource > >& oPair ) { return oPair.second == nullptr; } );
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

ResourceLoader::FontLoadCommand::FontLoadCommand( const std::filesystem::path& oFilePath, const FontResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
{
}

void ResourceLoader::FontLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	Array< uint8 > aAtlasData( FontResource::ATLAS_WIDTH * FontResource::ATLAS_HEIGHT );
	Array< stbtt_packedchar > aPackedCharacters( FontResource::GLYPH_COUNT );

	Array< uint8 > aFontData = ReadBinaryFile( m_oFilePath );

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

ResourceLoader::TextureLoadCommand::TextureLoadCommand( const std::filesystem::path& oFilePath, const TextureResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
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
	uint8* pData = stbi_load( m_oFilePath.string().c_str(), &iWidth, &iHeight, &iDepth, 0 );

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

ResourceLoader::ModelLoadCommand::ModelLoadCommand( const std::filesystem::path& oFilePath, const ModelResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
	, m_pScene( nullptr )
{
}

void ResourceLoader::ModelLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	aiScene* pSceneData = nullptr;

	const aiScene* pScene = g_pResourceLoader->m_oModelImporter.ReadFile( m_oFilePath.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace );
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
				TextureResPtr xTextureResource = g_pResourceLoader->LoadTexture( std::filesystem::path( sFile.C_Str() ) );
				m_xResource->m_aMaterials[ u ].m_xDiffuseTextureResource = xTextureResource;
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

	Array< Float3 > aVertices( uVertexCount );
	Array< Float3 > aNormals( uVertexCount );
	Array< Float3 > aTangents( uVertexCount );
	Array< Float3 > aBiTangents( uVertexCount );
	Array< Float2 > aUVs( uVertexCount );

	for( uint u = 0; u < uVertexCount; ++u )
	{
		aVertices[ u ] = Float3( pMesh->mVertices[ u ].x, pMesh->mVertices[ u ].y , pMesh->mVertices[ u ].z );
		aNormals[ u ] = Float3( pMesh->mNormals[ u ].x, pMesh->mNormals[ u ].y, pMesh->mNormals[ u ].z );
		aTangents[ u ] = Float3( pMesh->mTangents[ u ].x, pMesh->mTangents[ u ].y, pMesh->mTangents[ u ].z );
		aBiTangents[ u ] = Float3( pMesh->mBitangents[ u ].x, pMesh->mBitangents[ u ].y, pMesh->mBitangents[ u ].z );

		if( pMesh->mTextureCoords[ 0 ] != nullptr )
			aUVs[ u ] = Float2( pMesh->mTextureCoords[ 0 ][ u ].x, pMesh->mTextureCoords[ 0 ][ u ].y );
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
		.WithTangents( std::move( aTangents ) )
		.WithBiTangents( std::move( aBiTangents ) );

	if( pMesh->mMaterialIndex >= 0 && pMesh->mMaterialIndex < m_xResource->m_aMaterials.Count() )
		oMeshBuilder.WithMaterial( &m_xResource->m_aMaterials[ pMesh->mMaterialIndex ] );

	m_xResource->m_aMeshes.PushBack( oMeshBuilder.Build() );
}

ResourceLoader::ShaderLoadCommand::ShaderLoadCommand( const std::filesystem::path& oFilePath, const ShaderResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
	, m_eShaderType( ShaderType::UNDEFINED )
{
	if( oFilePath.extension() == ".vs" )
		m_eShaderType = ShaderType::VERTEX_SHADER;
	else if( m_oFilePath.extension() == ".ps" )
		m_eShaderType = ShaderType::PIXEL_SHADER;
}

void ResourceLoader::ShaderLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	std::string sContent = ReadTextFile( m_oFilePath );

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

ResourceLoader::TechniqueLoadCommand::TechniqueLoadCommand( const std::filesystem::path& oFilePath, const TechniqueResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
{
	std::filesystem::path oShaderPath = oFilePath;

	oShaderPath.replace_extension( ".vs" );
	m_aDependencies.PushBack( g_pResourceLoader->LoadShader( oShaderPath ).GetPtr() );

	oShaderPath.replace_extension( ".ps" );
	m_aDependencies.PushBack( g_pResourceLoader->LoadShader( oShaderPath ).GetPtr() );
}

void ResourceLoader::TechniqueLoadCommand::Load( std::unique_lock< std::mutex >& oLock )
{
	oLock.lock();
	m_eStatus = LoadCommandStatus::LOADED;
	oLock.unlock();
}

void ResourceLoader::TechniqueLoadCommand::OnFinished()
{
	switch( m_eStatus )
	{
	case ResourceLoader::LoadCommandStatus::FINISHED:
	{
		Array< const Shader* > aShaders;
		aShaders.Reserve( m_aDependencies.Count() );
		for( const StrongPtr< Resource >& pDependency : m_aDependencies )
		{
			const ShaderResource* pShaderResource = static_cast< const ShaderResource* >( pDependency.GetPtr() );
			ASSERT( pShaderResource->IsLoaded() );
			aShaders.PushBack( &pShaderResource->m_oShader );
		}
		m_xResource->m_oTechnique.Create( aShaders );
		
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

void ResourceLoader::TechniqueLoadCommand::OnDependenciesReady()
{
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
