#pragma once

#include "Core/ResourceLoader.h"

struct Scene
{
	Scene();

	bool OnLoading();

	ModelResPtr m_xCube;
};