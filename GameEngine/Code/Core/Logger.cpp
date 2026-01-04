#include "Logger.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <format>
#include <string>

static constexpr uint LOG_BUFFER_SIZE = 10 * 1024;

char* Logger::s_pLogBuffer = nullptr;
uint Logger::s_uLogBufferCursor = 0;

const char* Logger::s_aLogLevels[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR" };
Array< Logger* > Logger::s_aLoggers;

static OutputLogger sOutputLogger;
static FileLogger sFileLogger( "GameEngine.log" );

Logger::Logger()
{
	if( s_pLogBuffer == nullptr )
		s_pLogBuffer = new char[ LOG_BUFFER_SIZE ];

	RegisterLogger( this );
}

Logger::~Logger()
{
	if( s_aLoggers.Empty() && s_pLogBuffer != nullptr )
	{
		delete[] s_pLogBuffer;
		s_pLogBuffer = nullptr;
	}

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

	std::string sLog = ProduceLog( eLogLevel, sFile.substr( uFilePrefixLength ), iLine, sMessage );

	if( s_uLogBufferCursor + sLog.length() + 1 >= LOG_BUFFER_SIZE )
		Flush();

	strcpy( &s_pLogBuffer[ s_uLogBufferCursor ], sLog.c_str() );
	s_uLogBufferCursor += ( uint )sLog.length();
}

void Logger::Flush()
{
	for( Logger* pCurrentLogger : s_aLoggers )
		pCurrentLogger->FlushLogs();

	s_pLogBuffer[ 0 ] = '\0';
	s_uLogBufferCursor = 0;
}

// std::string Logger::ProduceLog( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage )
// {
// 	std::chrono::system_clock::time_point oNow = std::chrono::system_clock::now();
// 
// 	return std::format( "{} [{}] {}({}) : {}\n", oNow, s_aLogLevels[ eLogLevel ], sFile, iLine, sMessage );
// }

// std::string Logger::ProduceLog( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage )
// {
// 	std::chrono::system_clock::time_point oNow = std::chrono::system_clock::now();
// 
// 	std::chrono::year_month_day oYMD( std::chrono::floor< std::chrono::days >( oNow ) );
// 	std::chrono::hh_mm_ss oHMS( std::chrono::floor< std::chrono::milliseconds >( oNow - std::chrono::floor< std::chrono::days >( oNow ) ) );
// 	
// 	const int iYear = ( int )oYMD.year();
// 	const uint uMonth = ( uint )oYMD.month();
// 	const uint uDay = ( uint )oYMD.day();
// 	const int uHour = ( int )oHMS.hours().count();
// 	const int uMinute = ( int )oHMS.minutes().count();
// 	const int uSecond = ( int )oHMS.seconds().count();
// 	const int uMilliSecond = ( int )oHMS.subseconds().count();
// 
// 	return std::format( "{}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d} [{}] {}({}) : {}\n", iYear, uMonth, uDay, uHour, uMinute, uSecond, uMilliSecond, s_aLogLevels[ eLogLevel ], sFile, iLine, sMessage );
// }

std::string Logger::ProduceLog( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage )
{
	std::chrono::system_clock::time_point oNow = std::chrono::system_clock::now();

	std::chrono::year_month_day oYMD( std::chrono::floor< std::chrono::days >( oNow ) );
	std::chrono::hh_mm_ss oHMS( std::chrono::floor< std::chrono::milliseconds >( oNow - std::chrono::floor< std::chrono::days >( oNow ) ) );
	
	const int iYear = ( int )oYMD.year();
	const uint uMonth = ( uint )oYMD.month();
	const uint uDay = ( uint )oYMD.day();
	const int uHour = ( int )oHMS.hours().count();
	const int uMinute = ( int )oHMS.minutes().count();
	const int uSecond = ( int )oHMS.seconds().count();
	const int uMilliSecond = ( int )oHMS.subseconds().count();
	
	char sBuffer[ 1024 ];
	snprintf( sBuffer, 1024, "%d-%02d-%02d %02d:%02d:%02d.%03d [%s] %s(%d) : %s\n", iYear, uMonth, uDay, uHour, uMinute, uSecond, uMilliSecond, s_aLogLevels[ eLogLevel ], sFile.c_str(), iLine, sMessage.c_str() );
	return sBuffer;
}

void OutputLogger::FlushLogs()
{
	fputs( s_pLogBuffer, stdout );
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

	m_pFile = fopen( oFilePath.string().c_str(), "w+" );
}

FileLogger::~FileLogger()
{
	if( m_pFile != nullptr )
	{
		fclose( m_pFile );
		m_pFile = nullptr;
	}
}

void FileLogger::FlushLogs()
{
	fputs( s_pLogBuffer, m_pFile );
}
