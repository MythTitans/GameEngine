#include "FileUtils.h"

#include <fstream>

#include "Logger.h"

std::string ReadFileContent( const std::filesystem::path& oFilePath )
{
	std::string sFileContent;
	std::ifstream oFileStream( oFilePath );

	if( !oFileStream.is_open() )
	{
		LOG_ERROR( "Error reading file {}", oFilePath.string() );
		return "";
	}

	std::string sLine;
	while( !oFileStream.eof() )
	{
		std::getline( oFileStream, sLine );
		sFileContent.append( sLine + '\n' );
	}

	oFileStream.close();

	return sFileContent;
}
