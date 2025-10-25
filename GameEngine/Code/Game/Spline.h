#pragma once

#include "Core/Array.h"
#include "Component.h"

class SplineIterator
{
public:
	explicit SplineIterator( const Spline& oSpline );

	bool		MoveForward( const float fStepDistance, const float fSubStepDistance = 0.01f );

	glm::vec3	ComputePosition() const;
	glm::vec3	ComputeTangent() const;

private:
	const Spline&	m_oSpline;

	float			m_fRatio;
	float			m_fDistance;
	uint			m_uCPIndex;
};

class Spline
{
public:
	Spline() = default;
	explicit Spline( const Array< glm::vec3 >& aControlPoints );
	Spline( const Array< glm::vec3 >& aControlPoints, const Array< glm::vec3 >& aTangents );

	glm::vec3					ComputePosition( const float fRatio ) const;
	glm::vec3					ComputeTangent( const float fRatio ) const;
	float						ComputeDistance( const float fRatioA, const float fRatioB ) const;

	float						GetLength() const;

	void						RebuildTangents();
	void						RebuildDistances();

	Array< glm::vec3 >&			GetControlPoints();
	const Array< glm::vec3 >&	GetControlPoints() const;
	Array< glm::vec3 >&			GetTangents();
	const Array< glm::vec3 >&	GetTangents() const;
	const Array< float >&		GetDistances() const;
	const Array< float >&		GetCumulatedDistances() const;

private:
	Array< glm::vec3 >	m_aControlPoints;
	Array< glm::vec3 >	m_aTangents;
	Array< float >		m_aDistances;
	Array< float >		m_aCumulatedDistances;
};

class SplineComponent : public Component
{
public:
	friend class SplineCPComponent;

	SplineComponent( Entity* pEntity );

	void		Start() override;
	void		Update( const GameContext& oGameContext ) override;

	void		DisplayGizmos( const bool bSelected ) override;
#ifdef EDITOR
	bool		DisplayInspector() override;

	bool		IsEditing() const;
#endif

	Spline&		GetSpline();

private:
	PROPERTIES( SplineComponent );
	HIDDEN_PROPERTY( "Spline", m_oSpline, Spline );

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
