#include "Scene.h"

Scene::Scene()
	: m_xCube( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/cube.obj" ) ) )
{
}

bool Scene::OnLoading()
{
	return m_xCube->IsLoaded();
}
