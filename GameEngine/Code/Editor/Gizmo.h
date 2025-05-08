#pragma once

#include <glm/glm.hpp>

#include "Game/Component.h"

enum class GizmoType
{
	TRANSLATE,
	ROTATE,
	SCALE
};

enum class GizmoAxis
{
	X,
	Y,
	Z,
	XY,
	XZ,
	YZ
};

class GizmoComponent : public Component
{
public:
	explicit GizmoComponent( Entity* pEntity );

	void			Setup( const GizmoType eGizmoType, const GizmoAxis eGizmoAxis );
	void			Update( const GameContext& oGameContext ) override;

	const glm::vec3	GetColor() const;

	void			SetAnchor( Entity* pEntity );
	void			SetEditing( const bool bEditing );

	GizmoType		GetType() const;
	GizmoAxis		GetAxis() const;

	glm::mat4		GetWorldMatrix() const;

private:
	WeakPtr< Entity >	m_xAnchor;

	GizmoType			m_eGizmoType;
	GizmoAxis			m_eGizmoAxis;

	bool				m_bEditing;
};
