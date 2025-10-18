#pragma once

#include "Core/Intrusive.h"
#include "Core/Types.h"

class Entity;

class EntityHolder
{
public:
	EntityHolder();

	bool	DisplayInspector( const char* sName );

	void	SetEntity( const uint uEntityID );
	Entity* GetEntity();
	uint64	GetEntityID() const;

private:
	uint64				m_uEntityID;
	WeakPtr< Entity >	m_xEntity;
};
