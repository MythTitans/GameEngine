#pragma once

#include "Core/Array.h"
#include "Component.h"

class SplineComponent : public Component
{
public:
	friend class SplineCPComponent;

	SplineComponent( Entity* pEntity );

	void Start() override;

	void DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	bool DisplayInspector() override;
#endif

	glm::vec3 Evaluate( const float fDistance ) const;

private:
	PROPERTIES( SplineComponent );
	HIDDEN_PROPERTY( "Control points", m_aControlPoints, Array< glm::vec3 > );

	using SplineCPHandle = ComponentHandle< SplineCPComponent >;
	Array< SplineCPHandle >	m_aEditableControlPoints;
	bool					m_bEditing;
};

class SplineCPComponent : public Component
{
public:
	SplineCPComponent( Entity* pEntity );

	void DisplayGizmos( const bool bSelected ) override;

	void SetSplineData( SplineComponent* pSpline, const int iCPIndex );

private:
	using SplineHandle = ComponentHandle< SplineComponent >;
	SplineHandle	m_xSpline;
	int				m_iCPIndex;
};
