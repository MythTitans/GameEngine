#pragma once

#include "Array.h"

class Logger
{
public:
	enum LogLevel
	{
		LEVEL_TRACE,
		LEVEL_DEBUG,
		LEVEL_INFO,
		LEVEL_WARN,
		LEVEL_ERROR
	};

	Logger();
	~Logger();

	static void			RegisterLogger( Logger* pLogger );
	static void			UnRegisterLogger( Logger* pLogger );
	static void			Log( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage );
	static void			Flush();

protected:
	static char*			s_pLogBuffer;
	static uint				s_uLogBufferCursor;

private:
	static std::string	ProduceLog( const LogLevel eLogLevel, const std::string& sFile, const int iLine, const std::string& sMessage );

	virtual void		FlushLogs() = 0;

	static const char*		s_aLogLevels[ 5 ];
	static Array< Logger* >	s_aLoggers;
};

class OutputLogger : public Logger
{
private:
	void FlushLogs() override;
};

class FileLogger : public Logger
{
public:
	FileLogger( const std::string& sFileName );
	~FileLogger();

private:
	void FlushLogs() override;
	std::FILE* m_pFile;
};

#if LOG_LEVEL >= 5
#define LOG_TRACE( sMessage, ... ) Logger::Log( Logger::LEVEL_TRACE, __FILE__, __LINE__, std::format( sMessage, __VA_ARGS__ ) 
#else
#define LOG_TRACE( sMessage, ... ) ( void )( sMessage );
#endif

#if LOG_LEVEL >= 4
#define LOG_DEBUG( sMessage, ... ) Logger::Log( Logger::LEVEL_DEBUG, __FILE__, __LINE__, std::format( sMessage, __VA_ARGS__ ) )
#else
#define LOG_DEBUG( sMessage, ... ) ( void )( sMessage );
#endif

#if LOG_LEVEL >= 3
#define LOG_INFO( sMessage, ... ) Logger::Log( Logger::LEVEL_INFO, __FILE__, __LINE__, std::format( sMessage, __VA_ARGS__ ) )
#else
#define LOG_INFO( sMessage, ... ) ( void )( sMessage );
#endif

#if LOG_LEVEL >= 2
#define LOG_WARN( sMessage, ... ) Logger::Log( Logger::LEVEL_WARN, __FILE__, __LINE__, std::format( sMessage, __VA_ARGS__ ) )
#else
#define LOG_WARN( sMessage, ... ) ( void )( sMessage );
#endif

#if LOG_LEVEL >= 1
#define LOG_ERROR( sMessage, ... ) Logger::Log( Logger::LEVEL_ERROR, __FILE__, __LINE__, std::format( sMessage, __VA_ARGS__ ) )
#else
#define LOG_ERROR( sMessage, ... ) ( void )( sMessage );
#endif
