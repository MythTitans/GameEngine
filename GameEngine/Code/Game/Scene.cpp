#include "Scene.h"

Scene::Scene()
	//: m_xCube( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/cube.obj" ) ) )
	: m_xCube( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) ) )
	//: m_xCube( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/sphere.obj" ) ) )
{
}

bool Scene::OnLoading()
{
	return m_xCube->IsLoaded();
}
