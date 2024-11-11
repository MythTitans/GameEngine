#include "Logger.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

const char* Logger::s_aLogLevels[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR" };
Array< Logger* > Logger::s_aLoggers;

static OutputLogger sOutputLogger;
static FileLogger sFileLogger( "GameEngine.log" );

Logger::Logger()
{
	RegisterLogger( this );
}

Logger::~Logger()
{
	UnRegisterLogger( this );
}

void Logger::RegisterLogger( Logger* pLogger )
{
	for( Logger* pCurrentLogger : s_aLoggers )
	{
		if( pCurrentLogger == pLogger )
			return;
	}

	s_aLoggers.PushBack( pLogger );
}

void Logger::UnRegisterLogger( Logger* pLogger )
{
	for( uint u = 0; u < s_aLoggers.Count(); ++u )
	{
		if( s_aLoggers[ u ] == pLogger )
		{
			s_aLoggers.Remove( u );
			return;
		}
	}
}

void Logger::Log( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage )
{
	static const std::size_t uFilePrefixLength = std::filesystem::current_path().string().length() + 1;

	for( Logger* pCurrentLogger : s_aLoggers )
		pCurrentLogger->WriteLog( ProduceLog( eLogLevel, sFile.substr( uFilePrefixLength ), iLine, sMessage ) );
}

std::string Logger::ProduceLog( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage )
{
	return std::format( "{} [{}] {}({}) : {}\n", std::chrono::system_clock::now(), s_aLogLevels[ eLogLevel ], sFile, iLine, sMessage );
}

void OutputLogger::WriteLog( const std::string& sMessage )
{
	std::fputs( sMessage.c_str(), stdout );
}

FileLogger::FileLogger( const std::string& sFileName )
	: m_pFile( nullptr )
{
	const std::filesystem::path oFileName = std::filesystem::path( sFileName );
	const std::filesystem::path oFilePath = std::filesystem::current_path() / oFileName;

	for( uint u = 5; u > 1; --u )
	{
		std::filesystem::path oCurrentFilePath = oFilePath;
		oCurrentFilePath.replace_filename( oFileName.stem().string() + std::format( "_{}", u - 1 ) + oFileName.extension().string() );

		if( std::filesystem::exists( oCurrentFilePath ) )
		{
			std::filesystem::path oNewFilePath = oFilePath;
			oNewFilePath.replace_filename( oFileName.stem().string() + std::format( "_{}", u ) + oFileName.extension().string() );
			std::filesystem::rename( oCurrentFilePath, oNewFilePath );
		}
	}

	if( std::filesystem::exists( oFilePath ) )
	{
		std::filesystem::path oNewFilePath = oFilePath;
		oNewFilePath.replace_filename( oFileName.stem().string() + "_1" + oFileName.extension().string() );
		std::filesystem::rename( oFilePath, oNewFilePath );
	}

	m_pFile = std::fopen( oFilePath.string().c_str(), "w+" );
}

FileLogger::~FileLogger()
{
	if( m_pFile != nullptr )
	{
		std::fclose( m_pFile );
		m_pFile = nullptr;
	}
}

void FileLogger::WriteLog( const std::string& sMessage )
{
	std::fputs( sMessage.c_str(), m_pFile );
}
