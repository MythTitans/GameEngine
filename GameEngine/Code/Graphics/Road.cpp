#include "Road.h"

#include "Game/Entity.h"
#include "Game/ResourceLoader.h"
#include "Game/Spline.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"

float RoadComponent::MoveDistanceFromCurvature( const float fCurvature )
{
		if( fCurvature <= m_aCurvatures.Front() )
			return m_aDistances.Front();
		if( fCurvature >= m_aCurvatures.Back() )
			return m_aDistances.Back();

		uint uIndex = 0;
		while( m_aCurvatures[ uIndex ] < fCurvature )
			++uIndex;

		const float fRatio = ( fCurvature - m_aCurvatures[ uIndex - 1 ] ) / ( m_aCurvatures[ uIndex ] - m_aCurvatures[ uIndex - 1 ] );

		return glm::lerp( m_aDistances[ uIndex - 1 ], m_aDistances[ uIndex ], fRatio );

	//return glm::clamp( 1.f / ( 32.f * fCurvature ), 0.5f, 10.f );
}

Road::Road()
	: m_xRoad( g_pResourceLoader->LoadTechnique( "Shader/unlit.tech" ) )
{
}

void Road::Render( const RoadNode* pRoad, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xRoad->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	oTechnique.GetParameter( "modelViewProjection" ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pRoad->m_mMatrix ) );
	oTechnique.GetParameter( "diffuseColor" ).SetValue( glm::vec3( 1.f, 1.f, 1.f ) );

	g_pRenderer->SetTextureSlot( pRoad->m_oDiffuse, 0 );
	oTechnique.GetParameter( "diffuseMap" ).SetValue( 0 );

	g_pRenderer->DrawMesh( pRoad->m_oMesh );

	g_pRenderer->ClearTextureSlot( 0 );
}

bool Road::OnLoading()
{
	return m_xRoad->IsLoaded();
}

REGISTER_COMPONENT( RoadComponent, SplineComponent );

RoadComponent::RoadComponent( Entity* pEntity )
	: Component( pEntity )
{
	m_aCurvatures.Resize( 4 );
	m_aDistances.Resize( 4 );

	m_aCurvatures[ 0 ] = 0.001f;
	m_aCurvatures[ 1 ] = 0.005f;
	m_aCurvatures[ 2 ] = 0.02f;
	m_aCurvatures[ 3 ] = 0.1f;

	m_aDistances[ 0 ] = 10.f;
	m_aDistances[ 1 ] = 5.f;
	m_aDistances[ 2 ] = 2.5f;
	m_aDistances[ 3 ] = 0.5f;
}

void RoadComponent::Initialize()
{
	m_xSpline = GetComponent< SplineComponent >();
	//m_xTexture = g_pResourceLoader->LoadTexture( "Checker.png" );
	m_xTexture = g_pResourceLoader->LoadTexture( "RoadTest.jpg" );

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





	// TODO #eric temporary code
	uint u = 0;
	for( const float fCurve : m_xSpline->GetSpline().GetCurvatures() )
	{
		g_pDebugDisplay->DisplayText( std::format( "{} : {}", u, fCurve ), glm::vec4( 1.f, 0.5f, 0.f, 1.f ) );
		g_pDebugDisplay->DisplayText( std::format( "{} : {}", u, MoveDistanceFromCurvature( fCurve ) ), glm::vec4( 1.f, 0.f, 0.f, 1.f ) );

		++u;
	}

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

	ImGui::Checkbox( "Use curvature", &m_bUseCurvature );

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
		aLeftControlPoints[ u ] -= vNormal * 2.5f;
		aRightControlPoints[ u ] += vNormal * 2.5f;
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

// 	auto MoveDistanceFromCurvature = [ this ]( const float fCurvature )
// 	{
// 		if( m_bUseCurvature == false )
// 			return m_fDistance;
// 
// 		static const float fPowFactor = 2.f / glm::log2( 10.f );
// 
// 		// Original formula : 0.125 * pow(4, abs(log10(C)))
// 		// Transformed formula : 0.125 * pow(2, abs(log2(C)) * (2 / log2(10)))
// 		const float fClampedCurvature = glm::clamp( fCurvature, 0.001f, 0.1f );
// 		const float fCurvatureFactor = glm::abs( glm::log2( fClampedCurvature ) );
// 		const float fDistance = 0.125f * glm::pow( 2.f, fCurvatureFactor * fPowFactor );
// 
// 		return fDistance;
// 	};

// 	auto MoveDistanceFromCurvature = [ this ]( const float fCurvature )
// 	{
// 		if( m_bUseCurvature == false )
// 			return m_fDistance;
// 
// // 		if( fCurvature <= m_aCurvatures.Front() )
// // 			return m_aDistances.Front();
// // 		if( fCurvature >= m_aCurvatures.Back() )
// // 			return m_aDistances.Back();
// // 
// // 		uint uIndex = 0;
// // 		while( m_aCurvatures[ uIndex ] < fCurvature )
// // 			++uIndex;
// // 
// // 		const float fRatio = ( fCurvature - m_aCurvatures[ uIndex - 1 ] ) / ( m_aCurvatures[ uIndex ] - m_aCurvatures[ uIndex - 1 ] );
// // 
// // 		return glm::lerp( m_aDistances[ uIndex - 1 ], m_aDistances[ uIndex ], fRatio );
// 
// 		return glm::clamp( 1.f / ( 32.f * fCurvature ), 0.5f, 10.f );
// 	};

	glm::vec3 vPosition = oIt.ComputePosition();
	float fMoveDistance = MoveDistanceFromCurvature( oIt.ComputeCurvature() );

	aVertices.PushBack( oLeftSpline.ComputePosition( oIt.GetRatio() ) );
	aVertices.PushBack( oRightSpline.ComputePosition( oIt.GetRatio() ) );

	aUVs.PushBack( glm::vec2( 0.f, fLinearUV ) );
	aUVs.PushBack( glm::vec2( 1.f, 0.f ) );

	while( oIt.MoveForward( fMoveDistance, m_fTolerance ) )
	{
		fLinearUV += fMoveDistance / 5.f;

		vPosition = oIt.ComputePosition();
		fMoveDistance = MoveDistanceFromCurvature( oIt.ComputeCurvature() );

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
