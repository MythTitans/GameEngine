#include "StringUtils.h"

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
