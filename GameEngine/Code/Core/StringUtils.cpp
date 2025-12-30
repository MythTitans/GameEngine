#include "StringUtils.h"

#include <string>

#include "Core/Array.h"

void Replace( std::string& sString, const std::string_view sWhat, const std::string_view sBy )
{
	std::size_t uPos = 0;
	while( ( uPos = sString.find( sWhat, uPos ) ) != std::string::npos )
	{
		sString.replace( uPos, sWhat.length(), sBy );
		uPos += sBy.length();
	}
}

bool Contains( const std::string_view sString, const std::string_view sWhat )
{
	return sString.find( sWhat ) != std::string::npos;
}

Array< std::string > Split( const std::string& sString, const std::string& sDelimiter )
{
	Array< std::string > aParts;

	std::size_t uLastPos = 0;
	std::size_t uPos = 0;
	while( ( uPos = sString.find( sDelimiter, uLastPos ) ) != std::string::npos )
	{
		aParts.PushBack( sString.substr( uLastPos, uPos - uLastPos ) );
		uLastPos = uPos + sDelimiter.length();
	}

	if( uPos != uLastPos )
		aParts.PushBack( sString.substr( uLastPos ) );

	return aParts;
}
