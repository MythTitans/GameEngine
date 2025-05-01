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

	void			AttachToParent( Entity* pChild, Entity* pParent );
	void			DetachFromParent( Entity* pChild );

	void			SaveTestScene();
	void			LoadTestScene();

private:
	void			CreateInternalEntities();

	uint64			GenerateInternalID();
	uint64			GenerateID();

	uint64			m_uNextInternalID;
	uint64			m_uNextID;

	//std::unordered_map< uint64, Entity > m_mEntities;
	std::unordered_map< uint64, StrongPtr< Entity > > m_mEntities;
};