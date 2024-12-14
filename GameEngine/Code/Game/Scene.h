#pragma once

#include "Game/ResourceLoader.h"

struct Scene
{
	Scene();

	bool OnLoading();

	ModelResPtr m_xCube;
};