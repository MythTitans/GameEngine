#include "ResourceLoader.h"

#include <assimp/cimport.h>
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
#include "Core/StringUtils.h"
#include "Graphics/DebugDisplay.h"
#include "InputHandler.h"

template < typename T >
auto AssimpToGLM( const T& oAssimp )
{
	static_assert( false, "Not implemented" );
}

template <>
auto AssimpToGLM< aiVector3D >( const aiVector3D& vAssimp )
{
	return *reinterpret_cast< const glm::vec3* >( &vAssimp );
}

template <>
auto AssimpToGLM< aiQuaternion >( const aiQuaternion& qAssimp )
{
	return *reinterpret_cast< const glm::quat* >( &qAssimp );
}

template <>
auto AssimpToGLM< aiMatrix4x4 >( const aiMatrix4x4& mAssimp )
{
	return glm::transpose( *reinterpret_cast< const glm::mat4* >( &mAssimp ) );
}

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

const Texture& TextureResource::GetTexture() const
{
	return m_oTexture;
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

void ShaderResource::Destroy()
{
	m_oShader.Destroy();
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
	, m_bDisableUnusedResourcesDestruction( false )
	, m_bDisplayDebug( false )
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

TextureResPtr ResourceLoader::LoadTexture( const char* sFilePath, const bool bSRGB /*= false*/ )
{
	TextureResPtr& xTexturePtr = m_mTextureResources[ sFilePath ];
	if( xTexturePtr != nullptr )
		return xTexturePtr;

	xTexturePtr = new TextureResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aTextureLoadCommands.PushBack( TextureLoadCommand( sFilePath, xTexturePtr, bSRGB ) );

	return xTexturePtr;
}

TextureResPtr ResourceLoader::LoadTexture( const char* sFilePath, const uint8* pData, const uint uDataSize, const bool bSRGB /*= false */ )
{
	TextureResPtr& xTexturePtr = m_mTextureResources[ sFilePath ];
	if( xTexturePtr != nullptr )
		return xTexturePtr;

	xTexturePtr = new TextureResource();

	LOG_INFO( "Loading {}", sFilePath );

	int iWidth;
	int iHeight;
	int iDepth;
	uint8* pImageData = stbi_load_from_memory( pData, uDataSize, &iWidth, &iHeight, &iDepth, 0 );

	TextureLoadCommand oLoadCommand( sFilePath, xTexturePtr, bSRGB );
	oLoadCommand.m_eStatus = pImageData != nullptr ? LoadCommandStatus::LOADED : LoadCommandStatus::ERROR_READING;
	oLoadCommand.m_iWidth = iWidth;
	oLoadCommand.m_iHeight = iHeight;
	oLoadCommand.m_iDepth = iDepth;
	oLoadCommand.m_pData = pImageData;

	m_oPendingLoadCommands.m_aTextureLoadCommands.PushBack( oLoadCommand );

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

	if( m_bDisableUnusedResourcesDestruction == false )
		DestroyUnusedResources();
}

void ResourceLoader::ProcessLoadCommands()
{
	ProfilerBlock oBlock( "ProcessLoadCommands" );

	ProcessPendingLoadCommands();
}

void ResourceLoader::DisplayDebug()
{
	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_RESOURCES_DEBUG ) )
		m_bDisplayDebug = !m_bDisplayDebug;

	if( m_bDisplayDebug == false )
		return;

	ImGui::Begin( "Resources" );

	if( ImGui::CollapsingHeader( "Textures" ) )
	{
		ImGui::Text( "Textures count : %d", m_mTextureResources.size() );

		static bool bShowDetails = false;
		ImGui::Checkbox( "Show details", &bShowDetails );

		if( bShowDetails && ImGui::BeginTable( "Textures", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit ) )
		{
			ImGui::TableSetupColumn( "Texture" );
			ImGui::TableSetupColumn( "Infos" );
			ImGui::TableHeadersRow();

			auto TextureFormatToString = []( const TextureFormat eFormat ) {
				switch( eFormat )
				{
				case TextureFormat::RED:
					return "RED";
				case TextureFormat::RGB:
					return "RGB";
				case TextureFormat::RGBA:
					return "RGBA";
				default:
					return "Undefined";
				}
			};

			for( const auto& oPair : m_mTextureResources )
			{
				const Texture& oTexture = oPair.second->GetTexture();

				ImGui::TableNextRow();

 				ImGui::TableSetColumnIndex( 0 );
				const ImVec2 vFrom = ImGui::GetCursorScreenPos();
				ImGui::Image( oPair.second->GetTexture().GetID(), ImVec2( 128.f, 128.f ) );
				const ImVec2 vTo = ImVec2( vFrom.x + 128.f, vFrom.y + 128.f );
				if( ImGui::IsMouseHoveringRect( vFrom, vTo ) )
				{
					ImGui::BeginTooltip();
					ImGui::Image( oTexture.GetID(), ImVec2( 512.f, 512.f ) );
					ImGui::EndTooltip();
				}

				ImGui::TableSetColumnIndex( 1 );
				ImGui::Text( "ID : %s", oPair.first.c_str() );
				ImGui::Text( "Size : %d x %d", oTexture.GetWidth(), oTexture.GetHeight() );
				ImGui::Text( "Format : %s", TextureFormatToString( oTexture.GetFormat() ) );
				ImGui::Text( "References : %d", oPair.second->GetReferenceCount() - 1 );
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
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
		m_xResource->m_oAtlas.Create( TextureDesc( FontResource::ATLAS_WIDTH, FontResource::ATLAS_HEIGHT, TextureFormat::RED ).Data( m_aAtlasData.Data() ).GenerateMips() );
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

ResourceLoader::TextureLoadCommand::TextureLoadCommand( const char* sFilePath, const TextureResPtr& xResource, const bool bSRGB )
	: LoadCommand( sFilePath, xResource )
	, m_iWidth( 0 )
	, m_iHeight( 0 )
	, m_iDepth( 0 )
	, m_bSRGB( bSRGB )
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
		m_xResource->m_oTexture.Create( TextureDesc( m_iWidth, m_iHeight, m_iDepth == 3 ? TextureFormat::RGB : TextureFormat::RGBA ).Data( m_pData ).SRGB( m_bSRGB ).GenerateMips() );
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
		LoadAnimations();
		LoadSkeleton();
		LoadMaterials();
		LoadMeshes();
		aiReleaseImport( m_pScene );
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

	//m_xResource->m_aTechniqueResources = std::move( m_aDependencies );
	m_aDependencies.Clear();
}

void ResourceLoader::ModelLoadCommand::LoadAnimations()
{
	m_mNodeIndices[ "SkeletonRoot" ] = 0;

	m_xResource->m_aAnimations.Resize( m_pScene->mNumAnimations );

	for( uint uAnimation = 0; uAnimation < m_pScene->mNumAnimations; ++uAnimation )
	{
		const aiAnimation* pAnimation = m_pScene->mAnimations[ uAnimation ];

		Animation& oAnimation = m_xResource->m_aAnimations[ uAnimation ];
		oAnimation.m_sName = pAnimation->mName.C_Str();
		oAnimation.m_fDuration = ( float )( pAnimation->mDuration / pAnimation->mTicksPerSecond );
		oAnimation.m_aNodeAnimations.Resize( pAnimation->mNumChannels );

		for( uint uChannel = 0; uChannel < pAnimation->mNumChannels; ++uChannel )
		{
			const aiNodeAnim* pNodeAnimation = pAnimation->mChannels[ uChannel ];

			NodeAnimation& oNodeAnimation = oAnimation.m_aNodeAnimations[ uChannel ];
			const std::string sNodeName = pNodeAnimation->mNodeName.C_Str();

			oNodeAnimation.m_oPositionCurve.m_aTimes.Resize( pNodeAnimation->mNumPositionKeys );
			oNodeAnimation.m_oPositionCurve.m_aValues.Resize( pNodeAnimation->mNumPositionKeys );
			for( uint u = 0; u < pNodeAnimation->mNumPositionKeys; ++u )
			{
				oNodeAnimation.m_oPositionCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mPositionKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				oNodeAnimation.m_oPositionCurve.m_aValues[ u ] = AssimpToGLM( pNodeAnimation->mPositionKeys[ u ].mValue );
			}

			oNodeAnimation.m_oRotationCurve.m_aTimes.Resize( pNodeAnimation->mNumRotationKeys );
			oNodeAnimation.m_oRotationCurve.m_aValues.Resize( pNodeAnimation->mNumRotationKeys );
			for( uint u = 0; u < pNodeAnimation->mNumRotationKeys; ++u )
			{
				oNodeAnimation.m_oRotationCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mRotationKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				glm::quat qRotation = AssimpToGLM( pNodeAnimation->mRotationKeys[ u ].mValue );
				oNodeAnimation.m_oRotationCurve.m_aValues[ u ] = glm::quat( qRotation.x, qRotation.y, qRotation.z, qRotation.w );
			}

			oNodeAnimation.m_oScaleCurve.m_aTimes.Resize( pNodeAnimation->mNumScalingKeys );
			oNodeAnimation.m_oScaleCurve.m_aValues.Resize( pNodeAnimation->mNumScalingKeys );
			for( uint u = 0; u < pNodeAnimation->mNumScalingKeys; ++u )
			{
				oNodeAnimation.m_oScaleCurve.m_aTimes[ u ] = ( float )( pNodeAnimation->mScalingKeys[ u ].mTime / pAnimation->mTicksPerSecond );
				oNodeAnimation.m_oScaleCurve.m_aValues[ u ] = AssimpToGLM( pNodeAnimation->mScalingKeys[ u ].mValue );
			}

			oNodeAnimation.m_uMatrixIndex = FetchNodeIndex( sNodeName );
		}
	}
}

void ResourceLoader::ModelLoadCommand::LoadSkeleton()
{
	m_xResource->m_oSkeleton.m_uMatrixIndex = 0;

	m_xResource->m_aPoseMatrices.Resize( ( uint )m_mNodeIndices.size(), glm::mat4( 1.f ) );
	LoadSkeleton( m_pScene->mRootNode, m_xResource->m_oSkeleton );

	ASSERT( m_mNodeIndices.size() < MAX_BONE_COUNT );
	m_xResource->m_aSkinMatrices.Resize( glm::min( ( uint )m_mNodeIndices.size(), MAX_BONE_COUNT ), glm::mat4( 1.f ) );
}

void ResourceLoader::ModelLoadCommand::LoadMaterials()
{
	m_aMaterials.Resize( m_pScene->mNumMaterials );

	for( uint u = 0; u < m_pScene->mNumMaterials; ++u )
	{
		// 		TechniqueResPtr xTechniqueResource = g_pResourceLoader->LoadTechnique( "Shader/forward_opaque.tech" );
		// 		m_aDependencies.PushBack( xTechniqueResource.GetPtr() );

		const aiMaterial* pMaterial = m_pScene->mMaterials[ u ];

		aiColor3D oDiffuseColor;
		pMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, oDiffuseColor );
		m_aMaterials[ u ].m_vDiffuseColor = glm::vec3( oDiffuseColor.r, oDiffuseColor.g, oDiffuseColor.b );

		aiColor3D oSpecularColor;
		pMaterial->Get( AI_MATKEY_COLOR_SPECULAR, oSpecularColor );
		m_aMaterials[ u ].m_vSpecularColor = glm::vec3( oSpecularColor.r, oSpecularColor.g, oSpecularColor.b );

		aiColor3D oEmissiveColor;
		pMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, oEmissiveColor );
		m_aMaterials[ u ].m_vEmissiveColor = glm::vec3( oEmissiveColor.r, oEmissiveColor.g, oEmissiveColor.b );

		pMaterial->Get( AI_MATKEY_SHININESS, m_aMaterials[ u ].m_fShininess );

		if( pMaterial->GetTextureCount( aiTextureType_DIFFUSE ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str(), true );
				m_aMaterials[ u ].m_xDiffuseTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_NORMALS ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_NORMALS, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xNormalTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_SPECULAR ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_SPECULAR, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xSpecularTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}

		if( pMaterial->GetTextureCount( aiTextureType_EMISSIVE ) != 0 )
		{
			aiString sFile;
			if( pMaterial->GetTexture( aiTextureType_EMISSIVE, 0, &sFile ) == AI_SUCCESS )
			{
				TextureResPtr xTextureResource = LoadTexture( sFile.C_Str() );
				m_aMaterials[ u ].m_xEmissiveTextureResource = xTextureResource;
				m_aDependencies.PushBack( xTextureResource.GetPtr() );
			}
		}
	}
}

void ResourceLoader::ModelLoadCommand::LoadMeshes()
{
	aiNode* pRoot = m_pScene->mRootNode;

	m_xResource->m_aMeshes.Reserve( CountMeshes( pRoot ) );
	LoadMeshes( pRoot );
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

	Array< uint > aBonesCount( uVertexCount, 0 );
	Array< SkinData > aSkinData( uVertexCount );

	for( uint uBone = 0; uBone < pMesh->mNumBones; ++uBone )
	{
		const aiBone* pBone = pMesh->mBones[ uBone ];

		uint uBoneIndex = 0;
		const auto it = m_mNodeIndices.find( pBone->mName.C_Str() );
		ASSERT( it != m_mNodeIndices.cend() );
		if( it != m_mNodeIndices.cend() )
			uBoneIndex = it->second;
		
		m_xResource->m_aSkinMatrices[ uBoneIndex ] = AssimpToGLM( pBone->mOffsetMatrix );

		for( uint uWeight = 0; uWeight < pBone->mNumWeights; ++uWeight )
		{
			const aiVertexWeight& oVertexWeight = pBone->mWeights[ uWeight ];

			const uint uVertexIndex = oVertexWeight.mVertexId;
			const float fBoneWeight = ( float )oVertexWeight.mWeight;

			uint& uVertexBoneIndex = aBonesCount[ uVertexIndex ];
			ASSERT( uVertexBoneIndex < MAX_VERTEX_BONE_COUNT );
			aSkinData[ uVertexIndex ].m_aBones[ uVertexBoneIndex ] = uBoneIndex;
			aSkinData[ uVertexIndex ].m_aWeights[ uVertexBoneIndex ] = fBoneWeight;
			++uVertexBoneIndex;
		}
	}

	MeshBuilder oMeshBuilder = MeshBuilder( std::move( aVertices ), std::move( aIndices ) )
		.WithUVs( std::move( aUVs ) )
		.WithNormals( std::move( aNormals ) )
		.WithTangents( std::move( aTangents ) )
		.WithSkinData( std::move( aSkinData ) );

	if( pMesh->mMaterialIndex >= 0 && pMesh->mMaterialIndex < m_aMaterials.Count() )
	{
		const MaterialReference oMaterial = g_pMaterialManager->CreateMaterial( m_aMaterials[ pMesh->mMaterialIndex ] );
		oMeshBuilder.WithMaterial( oMaterial );
	}

	m_xResource->m_aMeshes.PushBack( oMeshBuilder.Build() );
}

void ResourceLoader::ModelLoadCommand::LoadSkeleton( aiNode* pNode, Skeleton& oParent )
{
	const std::string sNodeName = pNode->mName.C_Str();
	if( Contains( sNodeName, "$_Translation" ) || Contains( sNodeName, "$_PreRotation" ) || Contains( sNodeName, "$_Rotation" ) )
	{
		oParent.m_aChildren.Reserve( pNode->mNumChildren );
		for( uint u = 0; u < pNode->mNumChildren; ++u )
			LoadSkeleton( pNode->mChildren[ u ], oParent );
	}
	else
	{
		oParent.m_aChildren.PushBack( Skeleton() );

		Skeleton& oSkeleton = oParent.m_aChildren.Back();
		oSkeleton.m_aChildren.Reserve( pNode->mNumChildren );
		for( uint u = 0; u < pNode->mNumChildren; ++u )
			LoadSkeleton( pNode->mChildren[ u ], oSkeleton );

		const uint uNodeIndex = FetchNodeIndex( sNodeName );
		oSkeleton.m_uMatrixIndex = uNodeIndex;

		if( uNodeIndex >= m_xResource->m_aPoseMatrices.Count() )
			m_xResource->m_aPoseMatrices.Resize( uNodeIndex + 1 );

		m_xResource->m_aPoseMatrices[ uNodeIndex ] = AssimpToGLM( pNode->mTransformation );
	}
}

TextureResPtr ResourceLoader::ModelLoadCommand::LoadTexture( const std::string& sFileName, const bool bSRGB /*= false*/ )
{
	int uTextureIndex = -1;
	for( uint u = 0; u < m_pScene->mNumTextures; ++u )
	{
		if( sFileName == m_pScene->mTextures[ u ]->mFilename.C_Str() )
		{
			uTextureIndex = ( int )u;
			break;
		}
	}

	if( uTextureIndex != -1 )
	{
		aiTexture* pTexture = m_pScene->mTextures[ uTextureIndex ];

		std::string sInternalFileName = sFileName;
		const uint64 uOffset = sFileName.find( ".fbm/" );
		if( uOffset != std::string::npos )
			Replace( sInternalFileName, sInternalFileName.substr( 0, uOffset + 5 ), GetFilePath().string() + "@" );

		return g_pResourceLoader->LoadTexture( sInternalFileName.c_str(), ( uint8* )pTexture->pcData, pTexture->mWidth, bSRGB );
	}
	else
	{
		return g_pResourceLoader->LoadTexture( sFileName.c_str(), bSRGB );
	}
}

uint ResourceLoader::ModelLoadCommand::FetchNodeIndex( const std::string& sName )
{
	const auto it = m_mNodeIndices.find( sName );
	if( it != m_mNodeIndices.cend() )
	{
		return it->second;
	}

	const uint uIndex = ( uint )m_mNodeIndices.size();
	m_mNodeIndices[ sName ] = uIndex;
	return uIndex;
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
		m_aShaders.PushBack( sVertexShader );
		m_aShaders.PushBack( sPixelShader );
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
	case ResourceLoader::LoadCommandStatus::FINISHED:
		for( const std::string& sShader : m_aShaders )
			m_aDependencies.PushBack( g_pResourceLoader->LoadShader( sShader.c_str() ).GetPtr() );
		break;
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
