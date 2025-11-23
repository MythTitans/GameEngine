#include "Spline.h"

#include "Game/ResourceLoader.h"
#include "GameWorld.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Mesh.h"
#include "Graphics/Renderer.h"
#include "Entity.h"
#include "Math/GLMHelpers.h"

static void DisplayControlPointVisual( const Entity* pEntity, const glm::vec3& vColor )
{
	static Array< Mesh > s_aControlPointVisuals;
	static ModelResPtr s_xControlPointModel;
	static TechniqueResPtr s_xUnlitTechnique;

	if( s_aControlPointVisuals.Empty() )
	{
		if( s_xControlPointModel == nullptr )
			s_xControlPointModel = g_pResourceLoader->LoadModel( "sphere.obj" );

		if( s_xUnlitTechnique == nullptr )
			s_xUnlitTechnique = g_pResourceLoader->LoadTechnique( "Shader/unlit.tech" );

		if( s_xControlPointModel->IsLoaded() && s_xUnlitTechnique->IsLoaded() )
		{
			UnlitMaterialData oMaterialData;
			oMaterialData.m_vDiffuseColor = vColor;
			MaterialReference oMaterial = g_pMaterialManager->CreateMaterial( oMaterialData );

			s_aControlPointVisuals = s_xControlPointModel->GetMeshes();
			for( Mesh& oMesh : s_aControlPointVisuals )
				oMesh.SetMaterial( oMaterial );
		}
	}
	else
	{
		Transform oTransform = pEntity->GetWorldTransform();
		oTransform.SetScale( 0.25f, 0.25f, 0.25f );
		g_pRenderer->m_oVisualStructure.AddTemporaryNode( pEntity, oTransform.GetMatrixTRS(), s_aControlPointVisuals, s_xUnlitTechnique->GetTechnique() );
	}
}

SplineIterator::SplineIterator( const Spline& oSpline )
	: m_oSpline( oSpline )
	, m_fRatio( 0.f )
	, m_fDistance( 0.f )
	, m_uCPIndex( 0 )
{
}

bool SplineIterator::MoveForward( const float fStepDistance, const float fTolerance /*= 0.01f */ )
{
	const Array< glm::vec3 >& aControlPoints = m_oSpline.GetControlPoints();
	const Array< float >& aDistances = m_oSpline.GetDistances();
	const Array< float >& aCumulatedDistances = m_oSpline.GetCumulatedDistances();

	if( aControlPoints.Count() < 2 || m_uCPIndex >= aControlPoints.Count() - 1 )
		return false;

	const float fTargetDistance = m_fDistance + fStepDistance;

	while( aCumulatedDistances[ m_uCPIndex + 1 ] <= fTargetDistance )
	{
		++m_uCPIndex;
		if( m_uCPIndex >= aControlPoints.Count() - 1 )
			return false;

		m_fRatio = m_uCPIndex / ( float )( aControlPoints.Count() - 1 );
		m_fDistance = aCumulatedDistances[ m_uCPIndex ];
	}

	float fFromRatio = m_fRatio;
	float fToRatio = ( m_uCPIndex + 1 ) / ( float )( aControlPoints.Count() - 1 );
	while( glm::abs( fTargetDistance - m_fDistance ) >= fTolerance )
	{
		const float fMiddleRatio = fFromRatio + 0.5f * ( fToRatio - fFromRatio );
		const float fFoundDistance = m_fDistance + m_oSpline.ComputeDistance( fFromRatio, fMiddleRatio );

		if( fFoundDistance <= fTargetDistance )
		{
			fFromRatio = fMiddleRatio;
			m_fRatio = fMiddleRatio;
			m_fDistance = fFoundDistance;
		}
		else
		{
			fToRatio = fMiddleRatio;
		}
	}

	return true;
}

glm::vec3 SplineIterator::ComputePosition() const
{
	return m_oSpline.ComputePosition( m_fRatio );
}

glm::vec3 SplineIterator::ComputeTangent() const
{
	return m_oSpline.ComputeTangent( m_fRatio );
}

float SplineIterator::GetRatio() const
{
	return m_fRatio;
}

Spline::Spline( const Array< glm::vec3 >& aControlPoints )
	: m_aControlPoints( aControlPoints )
{
	RebuildTangents();
	RebuildDistances();
}

Spline::Spline( const Array< glm::vec3 >& aControlPoints, const Array< glm::vec3 >& aTangents )
	: m_aControlPoints( aControlPoints )
	, m_aTangents( aTangents )
{
	RebuildDistances();
}

glm::vec3 Spline::ComputePosition( const float fRatio ) const
{
	if( m_aControlPoints.Count() < 2 )
		return glm::vec3( 0.f, 0.f, 0.f );

	if( fRatio <= 0.f )
		return m_aControlPoints[ 0 ];

	if( fRatio >= 1.f )
		return m_aControlPoints.Back();

	const float fProgress = fRatio * ( m_aControlPoints.Count() - 1 );

	const uint uStartCP = ( int )fProgress;
	const uint uEndCP = uStartCP + 1;

	const float fBlend = fProgress - uStartCP;
	const float fBlendSquare = fBlend * fBlend;
	const float fBlendCube = fBlendSquare * fBlend;

	const float fStartCPFactor = 2 * fBlendCube - 3 * fBlendSquare + 1;
	const float fStartCPTangentFactor = fBlendCube - 2 * fBlendSquare + fBlend;
	const float fEndCPFactor = -2 * fBlendCube + 3 * fBlendSquare;
	const float fEndCPTangentFactor = fBlendCube - fBlendSquare;

	const glm::vec3& vStartCPPosition = m_aControlPoints[ uStartCP ];
	const glm::vec3& vEndCPPosition = m_aControlPoints[ uEndCP ];
	const glm::vec3& vStartCPTangent = m_aTangents[ uStartCP ];
	const glm::vec3& vEndCPTangent = m_aTangents[ uEndCP ];

	return glm::vec3(
		fStartCPFactor * vStartCPPosition.x + fStartCPTangentFactor * vStartCPTangent.x + fEndCPFactor * vEndCPPosition.x + fEndCPTangentFactor * vEndCPTangent.x,
		fStartCPFactor * vStartCPPosition.y + fStartCPTangentFactor * vStartCPTangent.y + fEndCPFactor * vEndCPPosition.y + fEndCPTangentFactor * vEndCPTangent.y,
		fStartCPFactor * vStartCPPosition.z + fStartCPTangentFactor * vStartCPTangent.z + fEndCPFactor * vEndCPPosition.z + fEndCPTangentFactor * vEndCPTangent.z );

	//return fStartCPFactor * vStartCPPosition + fStartCPTangentFactor * vStartCPTangent + fEndCPFactor * vEndCPPosition + fEndCPTangentFactor * vEndCPTangent;
}

glm::vec3 Spline::ComputeTangent( const float fRatio ) const
{
	if( m_aControlPoints.Count() < 2 )
		return glm::vec3( 1.f, 0.f, 0.f );

	if( fRatio <= 0.f )
		return m_aTangents[ 0 ];

	if( fRatio >= 1.f )
		return m_aTangents.Back();

	const float fProgress = fRatio * ( m_aTangents.Count() - 1 );

	const uint uStartCP = ( int )fProgress;
	const uint uEndCP = uStartCP + 1;

	const float fBlend = fProgress - uStartCP;
	const float fBlendSquare = fBlend * fBlend;
	const float fBlendCube = fBlendSquare * fBlend;

	const float fStartCPFactor = 6 * fBlendSquare - 6 * fBlend;
	const float fStartCPTangentFactor = 3 * fBlendSquare - 4 * fBlend + 1;
	const float fEndCPFactor = -6 * fBlendSquare + 6 * fBlend;
	const float fEndCPTangentFactor = 3 * fBlendSquare - 2 * fBlend;

	const glm::vec3 vStartCPPosition = m_aControlPoints[ uStartCP ];
	const glm::vec3 vEndCPPosition = m_aControlPoints[ uEndCP ];
	const glm::vec3 vStartCPTangent = m_aTangents[ uStartCP ];
	const glm::vec3 vEndCPTangent = m_aTangents[ uEndCP ];

	return fStartCPFactor * vStartCPPosition + fStartCPTangentFactor * vStartCPTangent + fEndCPFactor * vEndCPPosition + fEndCPTangentFactor * vEndCPTangent;
}

float Spline::ComputeDistance( const float fRatioA, const float fRatioB ) const
{
	float fLastDistance = glm::length( ComputePosition( fRatioB ) - ComputePosition( fRatioA ) );
	float fDistance = 0.f;
	uint uSubSegments = 2;

	bool bRefine = true;
	while( bRefine )
	{
		fDistance = 0.f;

		const float fSubSegmentRatio = ( fRatioB - fRatioA ) * ( 1.f / uSubSegments );

		for( uint uSubSegment = 0; uSubSegment < uSubSegments; ++uSubSegment )
		{
			const float fStartRatio = fRatioA + uSubSegment * fSubSegmentRatio;
			const float fEndRatio = fRatioA + ( uSubSegment + 1 ) * fSubSegmentRatio;

			fDistance += glm::length( ComputePosition( fEndRatio ) - ComputePosition( fStartRatio ) );
		}

		if( glm::abs( fDistance - fLastDistance ) < 0.01f )
			bRefine = false;

		fLastDistance = fDistance;
		uSubSegments *= 2;
	}

	return fDistance;
}

float Spline::GetLength() const
{
	if( m_aCumulatedDistances.Empty() )
		return 0.f;

	return m_aCumulatedDistances.Back();
}

void Spline::RebuildTangents()
{
	m_aTangents.Resize( m_aControlPoints.Count() );

	for( uint u = 0; u < m_aControlPoints.Count(); ++u )
	{
		if( u == 0 )
			m_aTangents[ u ] = ( m_aControlPoints[ u + 1 ] - m_aControlPoints[ u ] );
		else if( u == m_aControlPoints.Count() - 1 )
			m_aTangents[ u ] = ( m_aControlPoints[ u ] - m_aControlPoints[ u - 1 ] );
		else
			m_aTangents[ u ] = 0.5f * ( m_aControlPoints[ u + 1 ] - m_aControlPoints[ u - 1 ] );
	}
}

void Spline::RebuildDistances()
{
	m_aDistances.Resize( m_aControlPoints.Count() );
	m_aDistances[ 0 ] = 0.f;

	m_aCumulatedDistances.Resize( m_aControlPoints.Count() );
	m_aCumulatedDistances[ 0 ] = 0.f;

	const uint uSegments = m_aControlPoints.Count() - 1;
	const float fSegmentRatio = 1.f / uSegments;

	float fCumulatedDistance = 0.f;
	for( uint uSegment = 0; uSegment < uSegments; ++uSegment )
	{
		const float fDistance = ComputeDistance( uSegment * fSegmentRatio, ( uSegment + 1 ) * fSegmentRatio );
		fCumulatedDistance += fDistance;

		m_aDistances[ uSegment + 1 ] = fDistance;
		m_aCumulatedDistances[ uSegment + 1 ] = fCumulatedDistance;
	}
}

Array< glm::vec3 >& Spline::GetControlPoints()
{
	return m_aControlPoints;
}

const Array< glm::vec3 >& Spline::GetControlPoints() const
{
	return m_aControlPoints;
}

Array< glm::vec3 >& Spline::GetTangents()
{
	return m_aTangents;
}

const Array< glm::vec3 >& Spline::GetTangents() const
{
	return m_aTangents;
}

const Array< float >& Spline::GetDistances() const
{
	return m_aDistances;
}

const Array< float >& Spline::GetCumulatedDistances() const
{
	return m_aCumulatedDistances;
}

REGISTER_COMPONENT( SplineComponent );

SplineComponent::SplineComponent( Entity* pEntity )
	: Component( pEntity )
	, m_bEditing( false )
{
}

void SplineComponent::Start()
{
	for( const glm::vec3& vControlPoint : m_oSpline.GetControlPoints() )
	{
		Entity* pEntity = g_pGameWorld->CreateInternalEntity( std::format( "{}_CP", GetEntity()->GetName() ) );
		g_pGameWorld->AttachToParent( pEntity, GetEntity() );

		SplineCPComponent* pCP = g_pComponentManager->CreateComponent< SplineCPComponent >( pEntity );
		m_aEditableControlPoints.PushBack( pCP );
	}
}

void SplineComponent::Update( const GameContext& oGameContext )
{
	// TODO #eric could do this around modified CPs only (and only recompute cumulated)
	if( m_bEditing )
	{
		m_oSpline.RebuildTangents();
		m_oSpline.RebuildDistances();
	}
}

void SplineComponent::DisplayGizmos( const bool bSelected )
{
	uint uCPCount = m_oSpline.GetControlPoints().Count();

	if( uCPCount >= 2 )
	{
		const glm::mat4x3 mMatrixTR = GetEntity()->GetWorldTransform().GetMatrixTR();

		const uint uSteps = 20 * uCPCount;
		const float fStep = 1.f / uSteps;
		for( uint u = 0; u < uSteps; ++u )
		{
			const glm::vec3 vP1 = TransformPoint( mMatrixTR, m_oSpline.ComputePosition( u * fStep ) );
			const glm::vec3 vP2 = TransformPoint( mMatrixTR, m_oSpline.ComputePosition( ( u + 1 ) * fStep ) );

			g_pDebugDisplay->DisplayLine( vP1, vP2, glm::vec3( 1.f, 0.f, 0.f ) );
		}
	}
}

#ifdef EDITOR
bool SplineComponent::DisplayInspector()
{
	if( ImGui::Checkbox( "Edit spline", &m_bEditing ) )
	{
		if( m_bEditing )
		{
			for( uint u = 0; u < m_aEditableControlPoints.Count(); ++u )
				m_aEditableControlPoints[ u ]->SetSplineData( this, ( int )u );
		}
		else
		{
			for( uint u = 0; u < m_aEditableControlPoints.Count(); ++u )
				m_aEditableControlPoints[ u ]->SetSplineData( nullptr, -1 );
		}
	}

	if( m_bEditing )
	{
		if( ImGui::Button( "Add CP" ) )
		{
			m_oSpline.GetControlPoints().PushBack( glm::vec3( 0.f, 0.f, 0.f ) );
			m_oSpline.RebuildTangents();
			m_oSpline.RebuildDistances();

			Entity* pEntity = g_pGameWorld->CreateInternalEntity( std::format( "{}_CP", GetEntity()->GetName() ) );
			g_pGameWorld->AttachToParent( pEntity, GetEntity() );

			SplineCPComponent* pCP = g_pComponentManager->CreateComponent< SplineCPComponent >( pEntity );
			m_aEditableControlPoints.PushBack( pCP );

			pCP->SetSplineData( this, m_aEditableControlPoints.Count() - 1 );

			return true;
		}
	}

	return false;
}

bool SplineComponent::IsEditing() const
{
	return m_bEditing;
}

#endif

Spline& SplineComponent::GetSpline()
{
	return m_oSpline;
}

REGISTER_COMPONENT( SplineCPComponent );
SET_COMPONENT_PRIORITY_AFTER( SplineCPComponent, SplineComponent );

SplineCPComponent::SplineCPComponent( Entity* pEntity )
	: Component( pEntity )
	, m_iCPIndex( -1 )
{
}

void SplineCPComponent::DisplayGizmos( const bool bSelected )
{
	if( m_xSpline.IsValid() == false || m_iCPIndex < 0 )
		return;

	if( bSelected )
		m_xSpline->m_oSpline.GetControlPoints()[ m_iCPIndex ] = GetEntity()->GetPosition();
	else
		GetEntity()->SetPosition( m_xSpline->m_oSpline.GetControlPoints()[ m_iCPIndex ] );
	

	DisplayControlPointVisual( GetEntity(), glm::vec3( 0.f, 0.f, 1.f ) );
}

void SplineCPComponent::SetSplineData( SplineComponent* pSpline, const int iCPIndex )
{
	m_xSpline = pSpline;
	m_iCPIndex = iCPIndex;
}
