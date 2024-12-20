#pragma once

#include <unordered_map>

#include "Core/Array.h"
#include "Core/Intrusive.h"

class Entity;

class Scene
{
public:
	friend class Editor;

	Scene();

	Entity*			FindEntity( const uint64 uEntityID );
	const Entity*	FindEntity( const uint64 uEntityID ) const;

private:
	//std::unordered_map< uint64, Entity > m_mEntities;
	std::unordered_map< uint64, StrongPtr< Entity > > m_mEntities;
};