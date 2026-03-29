#include "ResourceLoader.h"

#define NOMINMAX
#include <Windows.h>

#include "Core/Common.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Core/stb_image.h"
#include "Core/StringUtils.h"
#include "Game/InputHandler.h"
#include "Graphics/DebugDisplay.h"
#include "LoadCommand.h"

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

TextureResPtr ResourceLoader::LoadTexture( const char* sFilePath, const bool bSRGB /*= false*/, const bool bUse16Bits /*= false*/ )
{
	TextureResPtr& xTexturePtr = m_mTextureResources[ sFilePath ];
	if( xTexturePtr != nullptr )
		return xTexturePtr;

	xTexturePtr = new TextureResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aTextureLoadCommands.PushBack( TextureLoadCommand( sFilePath, xTexturePtr, bSRGB, bUse16Bits ) );

	return xTexturePtr;
}

TextureResPtr ResourceLoader::LoadTexture( const char* sFilePath, const uint8* pData, const uint uDataSize, const bool bSRGB /*= false */, const bool bUse16Bits /*= false*/ )
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

	TextureLoadCommand oLoadCommand( sFilePath, xTexturePtr, bSRGB, bUse16Bits );
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

	Array< std::string > aFlags = Split( sFilePath, "|" );
	const std::string sRealFilePath = aFlags.Front();
	aFlags.PopFront();

	LOG_INFO( "Loading {}", sRealFilePath );
	m_oPendingLoadCommands.m_aShaderLoadCommands.PushBack( ShaderLoadCommand( sRealFilePath.c_str(), xShaderPtr, std::move( aFlags ) ) );

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

MaterialResPtr ResourceLoader::LoadMaterial( const char* sFilePath )
{
	MaterialResPtr& xMaterialPtr = m_mMaterialResources[ sFilePath ];
	if( xMaterialPtr != nullptr )
		return xMaterialPtr;

	xMaterialPtr = new MaterialResource();

	LOG_INFO( "Loading {}", sFilePath );
	m_oPendingLoadCommands.m_aMaterialLoadCommands.PushBack( MaterialLoadCommand( sFilePath, xMaterialPtr ) );

	return xMaterialPtr;
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
				case TextureFormat::R:
					return "R";
				case TextureFormat::R16:
					return "R16";
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
		if( oLoadCommand.m_eStatus != LoadCommandStatus::PENDING )
			continue;

		ProfilerBlock oResourceBlock( sCommandName, true );

		{
			ProfilerBlock oResourceBlock( "CheckResource", true );

			if( std::filesystem::exists( oLoadCommand.GetFilePath() ) == false )
			{
				oLock.lock();
				oLoadCommand.m_eStatus = LoadCommandStatus::NOT_FOUND;
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
		::Load( m_oProcessingLoadCommands.m_aMaterialLoadCommands, oLock, "LoadMaterial" );
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
		case LoadCommandStatus::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = LoadCommandStatus::NOT_FOUND;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case LoadCommandStatus::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = LoadCommandStatus::ERROR_READING;
			oLoadCommand.OnFinished();
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			++uFinishedCount;
			break;
		case LoadCommandStatus::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_sFilePath );
			oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
			oLoadCommand.OnFinished();
			if( oLoadCommand.HasDependencies() )
			{
				LOG_INFO( "Waiting dependencies for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = LoadCommandStatus::WAITING_DEPENDENCIES;
			}
			++uFinishedCount;
			break;
		case LoadCommandStatus::FINISHED:
		case LoadCommandStatus::WAITING_DEPENDENCIES:
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
		case LoadCommandStatus::WAITING_DEPENDENCIES:
			if( oLoadCommand.AllDependenciesLoaded() )
			{
				LOG_INFO( "Dependencies ready for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = LoadCommandStatus::FINISHED;
				oLoadCommand.OnDependenciesReady();
				++uFinishedCount;
			}
			else if( oLoadCommand.AnyDependencyFailed() )
			{
				LOG_ERROR( "Failed to load a dependency for {}", oLoadCommand.m_sFilePath );
				oLoadCommand.m_eStatus = LoadCommandStatus::ERROR_READING;
				oLoadCommand.OnDependenciesReady();
				++uFinishedCount;
			}
			break;
		case LoadCommandStatus::FINISHED:
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
	uFinishedCount += ::CheckFinishedProcessingLoadCommands( m_oProcessingLoadCommands.m_aMaterialLoadCommands );

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
	uFinishedCount += ::CheckWaitingDependenciesLoadCommands( m_oWaitingDependenciesLoadCommands.m_aMaterialLoadCommands );

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

uint ResourceLoader::LoadCommands::Count() const
{
	return m_aFontLoadCommands.Count() + m_aTextureLoadCommands.Count() + m_aModelLoadCommands.Count() + m_aShaderLoadCommands.Count() + m_aTechniqueLoadCommands.Count() + m_aMaterialLoadCommands.Count();
}

bool ResourceLoader::LoadCommands::Empty() const
{
	return m_aFontLoadCommands.Empty() && m_aTextureLoadCommands.Empty() && m_aModelLoadCommands.Empty() && m_aShaderLoadCommands.Empty() && m_aTechniqueLoadCommands.Empty() && m_aMaterialLoadCommands.Empty();
}

void ResourceLoader::LoadCommands::Grab( LoadCommands& oLoadCommands )
{
	m_aFontLoadCommands.Grab( oLoadCommands.m_aFontLoadCommands );
	m_aTextureLoadCommands.Grab( oLoadCommands.m_aTextureLoadCommands );
	m_aModelLoadCommands.Grab( oLoadCommands.m_aModelLoadCommands );
	m_aShaderLoadCommands.Grab( oLoadCommands.m_aShaderLoadCommands );
	m_aTechniqueLoadCommands.Grab( oLoadCommands.m_aTechniqueLoadCommands );
	m_aMaterialLoadCommands.Grab( oLoadCommands.m_aMaterialLoadCommands );
}

void ResourceLoader::LoadCommands::CopyWaitingDependencies( LoadCommands& oLoadCommands )
{
	m_aFontLoadCommands.Expand( oLoadCommands.m_aFontLoadCommands.Count() );
	m_aTextureLoadCommands.Expand( oLoadCommands.m_aTextureLoadCommands.Count() );
	m_aModelLoadCommands.Expand( oLoadCommands.m_aModelLoadCommands.Count() );
	m_aShaderLoadCommands.Expand( oLoadCommands.m_aShaderLoadCommands.Count() );
	m_aTechniqueLoadCommands.Expand( oLoadCommands.m_aTechniqueLoadCommands.Count() );
	m_aMaterialLoadCommands.Expand( oLoadCommands.m_aMaterialLoadCommands.Count() );

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

	for( const MaterialLoadCommand& oLoadCommand : oLoadCommands.m_aMaterialLoadCommands )
	{
		if( oLoadCommand.m_eStatus == LoadCommandStatus::WAITING_DEPENDENCIES )
			m_aMaterialLoadCommands.PushBack( oLoadCommand );
	}
}

void ResourceLoader::LoadCommands::Clear()
{
	m_aFontLoadCommands.Clear();
	m_aTextureLoadCommands.Clear();
	m_aModelLoadCommands.Clear();
	m_aShaderLoadCommands.Clear();
	m_aTechniqueLoadCommands.Clear();
	m_aMaterialLoadCommands.Clear();
}
