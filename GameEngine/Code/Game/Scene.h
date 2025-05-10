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

	void			Load( const std::string& sFilePath );
	void			Save( const std::string& sFilePath );

	Entity*			CreateEntity( const std::string& sName );
	Entity*			CreateEntity( const std::string& sName, const uint64 uID );

	void			RemoveEntity( const uint64 uEntityID );
	void			RemoveEntity( Entity* pEntity );

	Entity*			FindEntity( const uint64 uEntityID );
	const Entity*	FindEntity( const uint64 uEntityID ) const;

	void			AttachToParent( Entity* pChild, Entity* pParent );
	void			DetachFromParent( Entity* pChild );

private:
	void			CreateInternalEntities();
	Entity*			CreateInternalEntity( const std::string& sName );

	uint64			GenerateInternalID();
	uint64			GenerateID();
	void			UpdateID( const uint64 uID );

	uint64			m_uNextInternalID;
	uint64			m_uNextID;

	std::unordered_map< uint64, StrongPtr< Entity > >	m_mEntities;
};