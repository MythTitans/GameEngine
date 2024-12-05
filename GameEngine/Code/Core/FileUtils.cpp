#include "FileUtils.h"

#include <fstream>

#include "Logger.h"

std::string ReadTextFile( const std::filesystem::path& oFilePath )
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

Array< uint8 > ReadBinaryFile( const std::filesystem::path& oFilePath )
{
	std::ifstream oFileStream( oFilePath, std::ios::binary );

	Array< uint8 > aContent;

	if( !oFileStream.is_open() )
	{
		LOG_ERROR( "Error reading file {}", oFilePath.string() );
		return aContent;
	}

	oFileStream.seekg( 0, std::ios_base::end );
	const uint uLength = ( uint )oFileStream.tellg();
	oFileStream.seekg( 0, std::ios_base::beg );

	aContent.Resize( uLength );
	oFileStream.read( ( char* )( aContent.Data() ), uLength );

	oFileStream.close();

	return aContent;
}
