#pragma once

#include <unordered_map>

#include "Core/Array.h"
#include "Entity.h"

class Scene
{
public:
	Scene();

	Entity*			FindEntity( const uint64 uEntityID );
	const Entity*	FindEntity( const uint64 uEntityID ) const;

private:
	std::unordered_map< uint64, Entity > m_mEntities;
};