#include "Road.h"

#include "Game/Entity.h"
#include "Game/ResourceLoader.h"
#include "Game/Spline.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Renderer.h"

REGISTER_COMPONENT( RoadComponent, SplineComponent );

RoadComponent::RoadComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void RoadComponent::Initialize()
{
	m_xSpline = GetComponent< SplineComponent >();
	//m_xTexture = g_pResourceLoader->LoadTexture( "Checker.png" );
	m_xTexture = g_pResourceLoader->LoadTexture( "RoadTest.jpg", true );

	UnlitMaterialData oMaterialData;
	oMaterialData.m_vDiffuseColor = glm::vec3( 0.6f, 0.6f, 0.6f );
	oMaterialData.m_xDiffuseTextureResource = m_xTexture;
}

bool RoadComponent::IsInitialized() const
{
	return m_xTexture->IsLoading() == false;
}

void RoadComponent::Start()
{
	GenerateRoad();
	m_pRoadNode = g_pRenderer->m_oVisualStructure.AddRoad( GetEntity(), m_xTexture->GetTexture(), m_oMesh );
}

void RoadComponent::Update( const GameContext& oGameContext )
{
	if( m_xSpline.IsValid() && m_xSpline->IsEditing() )
		GenerateRoad();

	m_pRoadNode->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTR();
	m_pRoadNode->m_oMesh = m_oMesh;
}

void RoadComponent::Stop()
{
	g_pRenderer->m_oVisualStructure.RemoveRoad( m_pRoadNode );
}

void RoadComponent::Dispose()
{
	m_xTexture = nullptr;

	m_oMesh.Destroy();
}

void RoadComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
		g_pDebugDisplay->DisplayWireMesh( m_oMesh, GetEntity()->GetWorldTransform().GetMatrixTR(), glm::vec3( 1.f, 0.8f, 0.f ), false );
}

#ifdef EDITOR
bool RoadComponent::DisplayInspector()
{
	static float fDistance = m_fDistance;
	ImGui::InputFloat( "Distance", &fDistance );
	if( ImGui::IsItemDeactivatedAfterEdit() )
		m_fDistance = fDistance;

	static float fTolerance = m_fTolerance;
	ImGui::InputFloat( "Tolerance", &fTolerance );
	if( ImGui::IsItemDeactivatedAfterEdit() )
		m_fTolerance = fTolerance;

	if( ImGui::Button( "Generate road" ) )
		GenerateRoad();

	return false;
}

void RoadComponent::GenerateRoad()
{
	const Spline& oSpline = m_xSpline->GetSpline();

	const Array<glm::vec3>& aTangents = m_xSpline->GetSpline().GetTangents();

	Array<glm::vec3> aLeftControlPoints( m_xSpline->GetSpline().GetControlPoints() );
	Array<glm::vec3> aRightControlPoints( m_xSpline->GetSpline().GetControlPoints() );
	for( uint u = 0; u < aTangents.Count(); ++u )
	{
		glm::vec3 vNormal = glm::normalize( glm::cross( aTangents[ u ], glm::vec3( 0.f, 1.f, 0.f ) ) );
		aLeftControlPoints[ u ] -= vNormal * 5.f;
		aRightControlPoints[ u ] += vNormal * 5.f;
	}

	GenerateRoad( Spline( aLeftControlPoints ), Spline( aRightControlPoints ) );
}

void RoadComponent::GenerateRoad( const Spline& oLeftSpline, const Spline& oRightSpline )
{
	const Spline& oSpline = m_xSpline->GetSpline();

	const uint uEstimatedPoints = ( uint )( 2 * ( 1 + oSpline.GetLength() / m_fDistance ) );

	Array< glm::vec3 > aVertices;
	aVertices.Reserve( uEstimatedPoints );

	Array< glm::vec2 > aUVs;
	aUVs.Reserve( uEstimatedPoints );

	Array< GLuint > aIndices;
	aIndices.Reserve( 6 * ( ( uEstimatedPoints / 2 ) - 1 ) );

	float fLinearUV = 0.f;

	SplineIterator oIt( oSpline );

	glm::vec3 vPosition = oIt.ComputePosition();

	aVertices.PushBack( oLeftSpline.ComputePosition( oIt.GetRatio() ) );
	aVertices.PushBack( oRightSpline.ComputePosition( oIt.GetRatio() ) );

	aUVs.PushBack( glm::vec2( 0.f, fLinearUV ) );
	aUVs.PushBack( glm::vec2( 1.f, 0.f ) );

	while( oIt.MoveForward( m_fDistance, m_fTolerance ) )
	{
		fLinearUV += m_fDistance / 5.f;

		vPosition = oIt.ComputePosition();

		aVertices.PushBack( oLeftSpline.ComputePosition( oIt.GetRatio() ) );
		aVertices.PushBack( oRightSpline.ComputePosition( oIt.GetRatio() ) );

		aUVs.PushBack( glm::vec2( 0.f, fLinearUV ) );
		aUVs.PushBack( glm::vec2( 1.f, fLinearUV ) );
	}

	const uint uQuadCount = ( aVertices.Count() / 2 ) - 1;
	for( uint u = 0; u < uQuadCount; ++u )
	{
		uint uOffset = 2 * u;

		aIndices.PushBack( uOffset + 0 );
		aIndices.PushBack( uOffset + 1 );
		aIndices.PushBack( uOffset + 2 );

		aIndices.PushBack( uOffset + 2 );
		aIndices.PushBack( uOffset + 1 );
		aIndices.PushBack( uOffset + 3 );
	}

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();
}
#endif

REGISTER_COMPONENT( RoadTrenchComponent, RoadComponent );
SET_COMPONENT_PRIORITY_AFTER( RoadTrenchComponent, RoadComponent );

RoadTrenchComponent::RoadTrenchComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void RoadTrenchComponent::Initialize()
{
	m_xRoad = GetComponent< RoadComponent >();
}

void RoadTrenchComponent::Dispose()
{
	m_xRoad = nullptr;
}

const RoadNode* RoadTrenchComponent::GetRoadNode() const
{
	return m_xRoad->m_pRoadNode;
}
