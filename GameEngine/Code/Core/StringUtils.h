#pragma once

#include <string>

template < typename T >
class Array;

void					Replace( std::string& sString, const std::string_view sWhat, const std::string_view sBy );
bool					Contains( const std::string_view sString, const std::string_view sWhat );
Array< std::string >	Split( const std::string& sString, const std::string& sDelimiter );
