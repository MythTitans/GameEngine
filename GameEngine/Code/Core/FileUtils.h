#pragma once

#include <filesystem>
#include <string>

#include "Array.h"

std::string		ReadTextFile( const std::filesystem::path& oFilePath );
Array< uint8 >	ReadBinaryFile( const std::filesystem::path& oFilePath );