#include "Road.h"

#include "Entity.h"
#include "Graphics/DebugDisplay.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"
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

	UnlitMaterialData oMaterialData;
	oMaterialData.m_vDiffuseColor = glm::vec3( 0.6f, 0.6f, 0.6f );

	m_oMaterial = g_pMaterialManager->CreateMaterial( oMaterialData );
	m_oMesh.SetMaterial( m_oMaterial );
}

bool RoadComponent::IsInitialized() const
{
	return m_xTechnique->IsLoading() == false;
}

void RoadComponent::Update( const GameContext& oGameContext )
{
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

	const uint uEstimatedPoints = ( uint )( 2 * ( 1 + oSpline.GetLength() / m_fDistance ) );

	Array< glm::vec3 > aVertices;
	aVertices.Reserve( uEstimatedPoints );

	Array< GLuint > aIndices;
	aIndices.Reserve( 6 * ( ( uEstimatedPoints / 2 ) - 1 ) );

	bool bContinue = true;

	SplineIterator oIt( oSpline );
	while( bContinue )
	{
		const glm::vec3 vPosition = oIt.ComputePosition();
		const glm::vec3 vNormal = glm::normalize( glm::cross( oIt.ComputeTangent(), glm::vec3( 0.f, 1.f, 0.f ) ) );

		aVertices.PushBack( vPosition - 2.5f * vNormal );
		aVertices.PushBack( vPosition + 2.5f * vNormal );

		bContinue = oIt.MoveForward( m_fDistance, m_fTolerance );
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

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).Build();
	m_oMesh.SetMaterial( m_oMaterial );
}
#endif
