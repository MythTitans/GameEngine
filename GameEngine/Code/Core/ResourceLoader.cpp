#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Common.h"
#include "Logger.h"
#include "Profiler.h"

constexpr uint IO_THREAD_AFFINITY_MASK = 1 << 1;

ResourceLoader::ResourceLoader()
	: m_oIOThread( &ResourceLoader::Load, this )
{
	//SetThreadAffinityMask( m_oIOThread.native_handle(), IO_THREAD_AFFINITY_MASK );
	SetThreadDescription( m_oIOThread.native_handle(), L"IO thread" );
}

/*
void Texture::use(unsigned int textureUnit) const
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::unuse() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
*/

StrongPtr< TextureHolder > ResourceLoader::LoadTexture( const std::filesystem::path& oFilePath )
{
	StrongPtr< TextureHolder >& xHolderPtr = m_mTextureHolders[ oFilePath ];
	if( xHolderPtr != nullptr )
		return xHolderPtr;

	xHolderPtr = new TextureHolder( Texture() );

	LOG_INFO( "Loading {}", oFilePath.string() );
	m_aPendingLoadCommands.PushBack( TextureLoadCommand( oFilePath, xHolderPtr ) );

	return xHolderPtr;
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
}

void ResourceLoader::PostUpdate()
{
	ProfilerBlock oBlock( "ResourceLoader" );

	ProcessPendingLoadCommands();
}

void ResourceLoader::Load()
{
	while( true )
	{
		std::unique_lock oLock( m_oProcessingCommandsMutex );
		m_oProcessingCommandsConditionVariable.wait( oLock, [ this ]() { return m_aProcessingLoadCommands.Empty() == false; } );
		oLock.unlock();

		for( TextureLoadCommand& oLoadCommand : m_aProcessingLoadCommands )
		{
			if( oLoadCommand.m_eStatus != TextureLoadCommand::Status::PENDING )
				continue;

			if( std::filesystem::exists( oLoadCommand.m_oFilePath ) == false )
			{
				oLock.lock();
				oLoadCommand.m_eStatus = TextureLoadCommand::Status::NOT_FOUND;
				oLock.unlock();
				return;
			}

			int iWidth;
			int iHeight;
			int iDepth;
			uint8* pData = stbi_load( oLoadCommand.m_oFilePath.string().c_str(), &iWidth, &iHeight, &iDepth, 0 );

			oLock.lock();
			oLoadCommand.m_eStatus = pData != nullptr ? TextureLoadCommand::Status::LOADED : TextureLoadCommand::Status::ERROR_READING;
			oLoadCommand.m_iWidth = iWidth;
			oLoadCommand.m_iHeight = iHeight;
			oLoadCommand.m_iDepth = iDepth;
			oLoadCommand.m_pData = pData;
			oLock.unlock();
		}
	}
}

void ResourceLoader::ProcessPendingLoadCommands()
{
	ProfilerBlock oBlock( "Process load commands" );

	std::unique_lock oLock( m_oProcessingCommandsMutex );
	if( m_aProcessingLoadCommands.Empty() && m_aPendingLoadCommands.Empty() == false )
	{
		m_aProcessingLoadCommands = std::move( m_aPendingLoadCommands );
		oLock.unlock();

		m_oProcessingCommandsConditionVariable.notify_one();
	}
}

void ResourceLoader::CheckFinishedProcessingLoadCommands()
{
	ProfilerBlock oBlock( "Check finished load commands" );

	uint iFinishedCount = 0;

	std::unique_lock oLock( m_oProcessingCommandsMutex );
	for( uint u = 0; u < m_aProcessingLoadCommands.Count(); ++u )
	{
		TextureLoadCommand& oLoadCommand = m_aProcessingLoadCommands[ u ];
		switch( oLoadCommand.m_eStatus )
		{
		case TextureLoadCommand::Status::NOT_FOUND:
			LOG_ERROR( "File not found {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = TextureLoadCommand::Status::FINISHED;
			++iFinishedCount;
			break;
		case TextureLoadCommand::Status::ERROR_READING:
			LOG_ERROR( "Error reading file {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_eStatus = TextureLoadCommand::Status::FINISHED;
			++iFinishedCount;
			break;
		case TextureLoadCommand::Status::LOADED:
			LOG_INFO( "Loaded {}", oLoadCommand.m_oFilePath.string() );
			oLoadCommand.m_xTextureHolder->m_oResource.Create( oLoadCommand.m_iWidth, oLoadCommand.m_iHeight, TextureFormat::RGBA, oLoadCommand.m_pData ); // TODO #eric handle format
			stbi_image_free( oLoadCommand.m_pData );
			oLoadCommand.m_eStatus = TextureLoadCommand::Status::FINISHED;
			++iFinishedCount;
			break;
		case TextureLoadCommand::Status::FINISHED:
			++iFinishedCount;
			break;
		}
	}

	if( iFinishedCount == m_aProcessingLoadCommands.Count() )
		m_aProcessingLoadCommands.Clear();
}

void ResourceLoader::DestroyUnusedResources()
{
	ProfilerBlock oBlock( "Destroy unused resources" );

	// TODO #eric this is temporary code
	for( auto& oPair : m_mTextureHolders )
	{
		if( oPair.second->GetReferenceCount() == 1 )
		{
			oPair.second->GetResource().Destroy();
			oPair.second = nullptr;
		}
	}
}
