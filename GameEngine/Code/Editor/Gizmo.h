#pragma once

#ifdef EDITOR

#include <glm/glm.hpp>

#include "Core/Intrusive.h"
#include "Game/Component.h"

enum class GizmoType : uint8
{
	TRANSLATE,
	ROTATE,
	SCALE
};

enum class GizmoAxis : uint8
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

	glm::mat4x3		GetWorldMatrix() const;

private:
	WeakPtr< Entity >	m_xAnchor;

	GizmoType			m_eGizmoType;
	GizmoAxis			m_eGizmoAxis;

	bool				m_bEditing;
};

#endif
