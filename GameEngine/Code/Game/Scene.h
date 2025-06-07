#pragma once

#include <nlohmann/json_fwd.hpp>
#include <unordered_map>

#include "Core/Array.h"
#include "Core/Intrusive.h"

inline constexpr uint64 ENTITIES_START_ID = 1024;

class Entity;

class Scene
{
public:
	friend class Editor;

	Scene();

	void			Load( const nlohmann::json& oJsonContent );
	void			Save( nlohmann::json& oJsonContent );

	Entity*			CreateEntity( const std::string& sName );
	Entity*			CreateEntity( const std::string& sName, const uint64 uID );

	void			RemoveEntity( const uint64 uEntityID );
	void			RemoveEntity( Entity* pEntity );

	Entity*			FindEntity( const uint64 uEntityID );
	const Entity*	FindEntity( const uint64 uEntityID ) const;

	void			AttachToParent( Entity* pChild, Entity* pParent );
	void			DetachFromParent( Entity* pChild );

	void			Clear();

private:
	void			CreateInternalEntities();
	Entity*			CreateInternalEntity( const std::string& sName );

	uint64			GenerateInternalID();
	uint64			GenerateID();
	void			UpdateID( const uint64 uID );

	uint64			m_uNextInternalID;
	uint64			m_uNextID;

	std::unordered_map< uint64, StrongPtr< Entity > > m_mEntities;
};