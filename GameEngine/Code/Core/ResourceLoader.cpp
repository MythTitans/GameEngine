#include "ResourceLoader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Common.h"
#include "FileUtils.h"
#include "Graphics/Utils.h"
#include "Logger.h"
#include "Profiler.h"

Resource::Resource()
	: m_bLoaded( false )
{
}

bool Resource::IsLoaded() const
{
	return m_bLoaded;
}

Texture& TextureResource::GetTexture()
{
	return m_oTexture;
}

const Texture& TextureResource::GetTexture() const
{
	return m_oTexture;
}

Array< Mesh >& ModelResource::GetMeshes()
{
	return m_aMeshes;
}

const Array< Mesh >& ModelResource::GetMeshes() const
{
	return m_aMeshes;
}

Shader& ShaderResource::GetShader()
{
	return m_oShader;
}

const Shader& ShaderResource::GetShader() const
{
	return m_oShader;
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

void ResourceLoader::PreUpdate()
{
	ProfilerBlock oBlock( "ResourceLoader" );

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
}

void ResourceLoader::PostUpdate()
{
	ProfilerBlock oBlock( "ResourceLoader" );

	ProcessPendingLoadCommands();
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

		for( TextureLoadCommand& oLoadCommand : m_oProcessingLoadCommands.m_aTextureLoadCommands )
		{
			if( oLoadCommand.m_eStatus != LoadCommandStatus::PENDING )
				continue;

			ProfilerBlock oBlock( "LoadTextures", true );

			{
				ProfilerBlock oResourceBlock( "CheckResource", true );

				if( std::filesystem::exists( oLoadCommand.m_oFilePath ) == false )
				{
					oLock.lock();
					oLoadCommand.m_eStatus = LoadCommandStatus::NOT_FOUND;
					oLock.unlock();
					continue;
				}
			}
			
			{
				ProfilerBlock oResourceBlock( "LoadTexture", true );

				int iWidth;
				int iHeight;
				int iDepth;
				uint8* pData = stbi_load( oLoadCommand.m_oFilePath.string().c_str(), &iWidth, &iHeight, &iDepth, 0 );

				oLock.lock();
				oLoadCommand.m_eStatus = pData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
				oLoadCommand.m_iWidth = iWidth;
				oLoadCommand.m_iHeight = iHeight;
				oLoadCommand.m_iDepth = iDepth;
				oLoadCommand.m_pData = pData;
				oLock.unlock();
			}
		}

		for( ModelLoadCommand& oLoadCommand : m_oProcessingLoadCommands.m_aModelLoadCommands )
		{
			if( oLoadCommand.m_eStatus != LoadCommandStatus::PENDING )
				continue;

			ProfilerBlock oBlock( "LoadModels", true );

			{
				ProfilerBlock oResourceBlock( "CheckResource", true );

				if( std::filesystem::exists( oLoadCommand.m_oFilePath ) == false )
				{
					oLock.lock();
					oLoadCommand.m_eStatus = LoadCommandStatus::NOT_FOUND;
					oLock.unlock();
					continue;
				}
			}

			{
				ProfilerBlock oResourceBlock( "LoadModel", true );

				aiScene* pSceneData = nullptr;

				const aiScene* pScene = m_oModelImporter.ReadFile( oLoadCommand.m_oFilePath.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace );
				if( pScene != nullptr )
					pSceneData = m_oModelImporter.GetOrphanedScene();

				oLock.lock();
				oLoadCommand.m_eStatus = pSceneData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
				oLoadCommand.m_pScene = pSceneData;
				oLock.unlock();
			}
		}

		for( ShaderLoadCommand& oLoadCommand : m_oProcessingLoadCommands.m_aShaderLoadCommands )
		{
			if( oLoadCommand.m_eStatus != LoadCommandStatus::PENDING )
				continue;

			ProfilerBlock oBlock( "LoadShaders", true );

			{
				ProfilerBlock oResourceBlock( "CheckResource", true );

				if( std::filesystem::exists( oLoadCommand.m_oFilePath ) == false )
				{
					oLock.lock();
					oLoadCommand.m_eStatus = LoadCommandStatus::NOT_FOUND;
					oLock.unlock();
					continue;
				}
			}

			{
				ProfilerBlock oResourceBlock( "LoadShader", true );

				std::string sContent = ReadFileContent( oLoadCommand.m_oFilePath );

				oLock.lock();
				oLoadCommand.m_eStatus = LoadCommandStatus::LOADED;
				oLoadCommand.m_sShaderCode = std::move( sContent );
				oLock.unlock();
			}
		}
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

void ResourceLoader::CheckFinishedProcessingLoadCommands()
{
	ProfilerBlock oBlock( "CheckFinishedLoadCommands" );

	uint iFinishedCount = 0;

	std::unique_lock oLock( m_oProcessingCommandsMutex );
	for( uint u = 0; u < m_oProcessingLoadCommands.m_aTextureLoadCommands.Count(); ++u )
	{
		TextureLoadCommand& oLoadCommand = m_oProcessingLoadCommands.m_aTextureLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case LoadCommandStatus::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.OnLoaded();
			stbi_image_free( oLoadCommand.m_pData );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::FINISHED:
			++iFinishedCount;
			break;
		}
	}

	for( uint u = 0; u < m_oProcessingLoadCommands.m_aModelLoadCommands.Count(); ++u )
	{
		ModelLoadCommand& oLoadCommand = m_oProcessingLoadCommands.m_aModelLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case LoadCommandStatus::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.OnLoaded();
			delete oLoadCommand.m_pScene;
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::FINISHED:
			++iFinishedCount;
			break;
		}
	}

	for( uint u = 0; u < m_oProcessingLoadCommands.m_aShaderLoadCommands.Count(); ++u )
	{
		ShaderLoadCommand& oLoadCommand = m_oProcessingLoadCommands.m_aShaderLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case LoadCommandStatus::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.OnLoaded();
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
			break;
		case LoadCommandStatus::FINISHED:
			++iFinishedCount;
			break;
		}
	}

	// TODO #eric should have a way to check that shaders are not in error, otherwise the loading will hang forever
	for( uint u = 0; u < m_oProcessingLoadCommands.m_aTechniqueLoadCommands.Count(); ++u )
	{
		TechniqueLoadCommand& oLoadCommand = m_oProcessingLoadCommands.m_aTechniqueLoadCommands[ u ];

		bool bLoaded = true;
		for( const ShaderResPtr& xShaderResource : oLoadCommand.m_aShaderResources )
		{
			if( xShaderResource->m_bLoaded == false )
			{
				bLoaded = false;
				break;
			}
		}

		if( bLoaded )
		{
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.OnLoaded();
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++iFinishedCount;
		}
	}

	if( iFinishedCount == m_oProcessingLoadCommands.Count() )
		m_oProcessingLoadCommands.Clear();
}

void ResourceLoader::DestroyUnusedResources()
{
	ProfilerBlock oBlock( "DestroyUnusedResources" );

	// TODO #eric this is temporary code
	for( auto& oPair : m_mTextureResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			oPair.second->GetTexture().Destroy();
			oPair.second = nullptr;
		}
	}

	// TODO #eric this is temporary code
	for( auto& oPair : m_mModelResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			for( Mesh& oMesh : oPair.second->GetMeshes() )
				oMesh.Destroy();
			oPair.second = nullptr;
		}
	}

	// TODO #eric this is temporary code
	for( auto& oPair : m_mTechniqueResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			oPair.second->GetTechnique().Destroy();
			oPair.second = nullptr;
		}
	}

	// TODO #eric this is temporary code
	for( auto& oPair : m_mShaderResources )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			oPair.second->GetShader().Destroy();
			oPair.second = nullptr;
		}
	}
}

ResourceLoader::TextureLoadCommand::TextureLoadCommand( const std::filesystem::path& oFilePath, const TextureResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_iDepth( 0 )
	, m_pData( nullptr )
{
}

void ResourceLoader::TextureLoadCommand::OnLoaded()
{
	m_xResource->m_oTexture.Create( m_iWidth, m_iHeight, TextureFormat::RGBA, m_pData ); // TODO #eric handle format

	m_xResource->m_bLoaded = true;
}

ResourceLoader::ModelLoadCommand::ModelLoadCommand( const std::filesystem::path& oFilePath, const ModelResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
	, m_pScene( nullptr )
{
}

void ResourceLoader::ModelLoadCommand::OnLoaded()
{
	aiNode* pRoot = m_pScene->mRootNode;

	m_xResource->m_aMeshes.Reserve( CountMeshes( pRoot ) );

	LoadMeshes( pRoot );

	m_xResource->m_bLoaded = true;
}

uint ResourceLoader::ModelLoadCommand::CountMeshes( aiNode* pNode )
{
	uint uCount = pNode->mNumMeshes;

	for( uint u = 0; u < pNode->mNumChildren; ++u )
		uCount += CountMeshes( pNode->mChildren[ u ] );

	return uCount;
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
	Array< Float3  > aNormals( uVertexCount );
	Array< Float3  > aTangents( uVertexCount );
	Array< Float3  > aBiTangents( uVertexCount );
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

 	m_xResource->m_aMeshes.PushBack( Mesh() );
 	m_xResource->m_aMeshes.Back().Create( aVertices, aUVs, aNormals, aTangents, aBiTangents, aIndices );
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

void ResourceLoader::ShaderLoadCommand::OnLoaded()
{
	m_xResource->m_oShader.Create( m_sShaderCode, m_eShaderType );

	m_xResource->m_bLoaded = true;
}

ResourceLoader::TechniqueLoadCommand::TechniqueLoadCommand( const std::filesystem::path& oFilePath, const TechniqueResPtr& xResource )
	: LoadCommand( oFilePath, xResource )
{
	std::filesystem::path oShaderPath = oFilePath;

	oShaderPath.replace_extension( ".vs" );
	m_aShaderResources.PushBack( g_pResourceLoader->LoadShader( oShaderPath ) );

	oShaderPath.replace_extension( ".ps" );
	m_aShaderResources.PushBack( g_pResourceLoader->LoadShader( oShaderPath ) );
}

void ResourceLoader::TechniqueLoadCommand::OnLoaded()
{
	Array< const Shader* > aShaders;
	aShaders.Reserve( m_aShaderResources.Count() );
	for( const ShaderResPtr pShader : m_aShaderResources )
	{
		ASSERT( pShader->IsLoaded() );
		aShaders.PushBack( &pShader->m_oShader );
	}
	m_xResource->m_oTechnique.Create( aShaders );

	m_xResource->m_aShaderResources = std::move( m_aShaderResources );
	m_xResource->m_bLoaded = true;
}

uint ResourceLoader::LoadCommands::Count() const
{
	return m_aTextureLoadCommands.Count() + m_aModelLoadCommands.Count() + m_aShaderLoadCommands.Count() + m_aTechniqueLoadCommands.Count();
}

bool ResourceLoader::LoadCommands::Empty() const
{
	return m_aTextureLoadCommands.Empty() && m_aModelLoadCommands.Empty() && m_aShaderLoadCommands.Empty() && m_aTechniqueLoadCommands.Empty();
}

void ResourceLoader::LoadCommands::Grab( LoadCommands& oLoadCommands )
{
	m_aTextureLoadCommands.Grab( oLoadCommands.m_aTextureLoadCommands );
	m_aModelLoadCommands.Grab( oLoadCommands.m_aModelLoadCommands );
	m_aShaderLoadCommands.Grab( oLoadCommands.m_aShaderLoadCommands );
	m_aTechniqueLoadCommands.Grab( oLoadCommands.m_aTechniqueLoadCommands );
}

void ResourceLoader::LoadCommands::Clear()
{
	m_aTextureLoadCommands.Clear();
	m_aModelLoadCommands.Clear();
	m_aShaderLoadCommands.Clear();
	m_aTechniqueLoadCommands.Clear();
}
