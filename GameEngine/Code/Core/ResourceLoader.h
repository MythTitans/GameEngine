#pragma once

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Core/Array.h"
#include "Core/Intrusive.h"
#include "Graphics/Texture.h"

template < typename Resource >
class ResourceHolder : public Intrusive
{
public:
	friend class ResourceLoader;

	explicit ResourceHolder( const Resource& oResource )
		: m_oResource( oResource )
	{
	}

	void SetResource( const Resource& oResource )
	{
		m_oResource = oResource;
	}

	Resource& GetResource()
	{
		return m_oResource;
	}

	const Resource& GetResource() const
	{
		return m_oResource;
	}

private:
	Resource m_oResource;
};

using TextureHolder = ResourceHolder< Texture >;

class ResourceLoader
{
public:
	ResourceLoader();

	StrongPtr< TextureHolder >	LoadTexture( const std::filesystem::path& oFilePath );

	void						PreUpdate();
	void						Update();
	void						PostUpdate();

private:
	void						Load();
	void						ProcessPendingLoadCommands();
	void						CheckFinishedProcessingLoadCommands();
	void						DestroyUnusedResources();

	struct TextureLoadCommand
	{
		enum class Status : uint8
		{
			PENDING,
			LOADING,
			LOADED,
			FINISHED, 
			NOT_FOUND,
			ERROR_READING
		};

		TextureLoadCommand( const std::filesystem::path& oFilePath, const StrongPtr< TextureHolder >& xTextureHolder )
			: m_oFilePath( oFilePath )
			, m_xTextureHolder( xTextureHolder )
			, m_eStatus( Status::PENDING )
		{
		}

		std::filesystem::path		m_oFilePath;
		StrongPtr< TextureHolder >	m_xTextureHolder;
		Status						m_eStatus;

		int							m_iWidth;
		int							m_iHeight;
		int							m_iDepth;
		uint8*						m_pData;
	};

	using TextureHolderMap = std::unordered_map< std::filesystem::path, StrongPtr< TextureHolder > >;
	using TextureLoadCommands = Array< TextureLoadCommand >;

	TextureHolderMap		m_mTextureHolders;

	TextureLoadCommands		m_aPendingLoadCommands;
	TextureLoadCommands		m_aProcessingLoadCommands;

	std::mutex				m_oProcessingCommandsMutex;
	std::condition_variable m_oProcessingCommandsConditionVariable;
	std::thread				m_oIOThread;
};