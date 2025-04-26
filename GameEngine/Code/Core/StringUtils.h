#pragma once

#include <string>

void Replace( std::string& sString, const std::string_view sWhat, const std::string_view sBy );
bool Contains( const std::string_view sString, const std::string_view sWhat );
