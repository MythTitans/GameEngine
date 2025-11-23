#include "Road.h"

#include "Entity.h"
#include "GameWorld.h"
#include "Graphics/Renderer.h"
#include "Spline.h"

REGISTER_COMPONENT( RoadComponent, SplineComponent );

RoadComponent::RoadComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void RoadComponent::Initialize()
{
	m_xSpline = GetComponent< SplineComponent >();
	m_xTechnique = g_pResourceLoader->LoadTechnique( "Shader/unlit.tech" );
	//m_xTexture = g_pResourceLoader->LoadTexture( "Checker.png" );
	m_xTexture = g_pResourceLoader->LoadTexture( "RoadTest.jpg" );

	UnlitMaterialData oMaterialData;
	oMaterialData.m_vDiffuseColor = glm::vec3( 0.6f, 0.6f, 0.6f );
	oMaterialData.m_xDiffuseTextureResource = m_xTexture;

	m_oMaterial = g_pMaterialManager->CreateMaterial( oMaterialData );
	m_oMesh.SetMaterial( m_oMaterial );
}

bool RoadComponent::IsInitialized() const
{
	return m_xTechnique->IsLoading() == false && m_xTexture->IsLoading() == false;
}

void RoadComponent::Update( const GameContext& oGameContext )
{
// 	bool bContinue = true;
// 
// 	SplineIterator oIt( m_xSpline->GetSpline() );
// 	while( bContinue )
// 	{
// 		const glm::vec3 vPosition = oIt.ComputePosition();
// 		const glm::vec3 vNormal = glm::normalize( glm::cross( oIt.ComputeTangent(), glm::vec3( 0.f, 1.f, 0.f ) ) );
// 
// 		g_pDebugDisplay->DisplaySphere( TransformPoint( GetEntity()->GetWorldTransform().GetMatrixTR(), vPosition ), 0.1f, glm::vec3( 1.f, 0.5f, 0.f ) );
// 
// 		bContinue = oIt.MoveForward( m_fDistance, m_fTolerance );
// 	}

	if( m_xSpline.IsValid() && m_xSpline->IsEditing() )
		GenerateRoad();

	// TODO #eric temporary code, use a dedicated node in the visual structure
	Entity* pEntity = GetEntity();
	const glm::mat4x3 mMatrixTR = pEntity->GetWorldTransform().GetMatrixTR();
	g_pRenderer->m_oVisualStructure.AddTemporaryNode( pEntity, mMatrixTR, Array<Mesh>( 1, m_oMesh ), m_xTechnique->GetTechnique() );
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
	m_oMesh.SetMaterial( m_oMaterial );
}
#endif
