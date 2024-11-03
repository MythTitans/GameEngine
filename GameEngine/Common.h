#pragma once

#include <cassert>

#include "Types.h"

#define ASSERT( expression ) assert( expression || [](){ __debugbreak(); return false; }() )