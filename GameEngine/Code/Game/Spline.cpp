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

REGISTER_COMPONENT( SplineComponent );

SplineComponent::SplineComponent( Entity* pEntity )
	: Component( pEntity )
	, m_bEditing( false )
{
}

void SplineComponent::Start()
{
	for( const glm::vec3& vControlPoint : m_aControlPoints )
	{
		Entity* pEntity = g_pGameWorld->CreateInternalEntity( std::format( "{}_CP", GetEntity()->GetName() ) );
		g_pGameWorld->AttachToParent( pEntity, GetEntity() );

		SplineCPComponent* pCP = g_pComponentManager->CreateComponent< SplineCPComponent >( pEntity );
		m_aEditableControlPoints.PushBack( pCP );
	}
}

void SplineComponent::DisplayGizmos( const bool bSelected )
{
	if( m_aControlPoints.Count() >= 2 )
	{
		const glm::mat4 mMatrixTR = ToMat4( GetEntity()->GetWorldTransform().GetMatrixTR() );

		const uint uSubdivisions = 20;
		const uint uSteps = ( m_aControlPoints.Count() - 1 ) * uSubdivisions;
		const float fStep = ( float )( m_aControlPoints.Count() - 1 ) / uSteps;
		for( uint u = 0; u < uSteps; ++u )
		{
			const glm::vec4 vP1 = mMatrixTR * glm::vec4( Evaluate( u * fStep ), 1.f );
			const glm::vec4 vP2 = mMatrixTR * glm::vec4( Evaluate( ( u + 1 ) * fStep ), 1.f );

			g_pDebugDisplay->DisplayLine( glm::vec3( vP1 ), glm::vec3( vP2 ), glm::vec3( 1.f, 0.f, 0.f ) );
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

	if( m_bEditing && ImGui::Button( "Add CP" ) )
	{
		m_aControlPoints.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );

		Entity* pEntity = g_pGameWorld->CreateInternalEntity( std::format( "{}_CP", GetEntity()->GetName() ) );
		g_pGameWorld->AttachToParent( pEntity, GetEntity() );

		SplineCPComponent* pCP = g_pComponentManager->CreateComponent< SplineCPComponent >( pEntity );
		m_aEditableControlPoints.PushBack( pCP );

		pCP->SetSplineData( this, m_aEditableControlPoints.Count() - 1 );

		return true;
	}

	return false;
}
#endif

glm::vec3 SplineComponent::Evaluate( const float fDistance ) const
{
	if( m_aControlPoints.Empty() )
		return glm::vec3( 0.f, 0.f, 0.f );

	if( fDistance <= 0.f )
		return m_aControlPoints[ 0 ];

	if( fDistance >= m_aControlPoints.Count() - 1 )
		return m_aControlPoints.Back();

	const int iStartCP = ( int )fDistance;
	const int iEndCP = iStartCP + 1;

	const float fBlend = fDistance - iStartCP;
	const float fBlendSquare = fBlend * fBlend;
	const float fBlendCube = fBlendSquare * fBlend;

	const float fStartCPFactor = 2 * fBlendCube - 3 * fBlendSquare + 1;
	const float fStartCPTangentFactor = fBlendCube - 2 * fBlendSquare + fBlend;
	const float fEndCPFactor = -2 * fBlendCube + 3 * fBlendSquare;
	const float fEndCPTangentFactor = fBlendCube - fBlendSquare;

	// TODO #eric precompute tangents
	auto ComputeTangent = [ this ]( const int iCPIndex) {
		if( iCPIndex == 0 )
			return 0.5f * ( m_aControlPoints[ iCPIndex + 1 ] - m_aControlPoints[ iCPIndex ] );
		if( iCPIndex == m_aControlPoints.Count() - 1 )
			return 0.5f * ( m_aControlPoints[ iCPIndex ] - m_aControlPoints[ iCPIndex - 1 ] );

		return 0.5f * ( m_aControlPoints[ iCPIndex + 1 ] - m_aControlPoints[ iCPIndex - 1 ] );
	};

	const glm::vec3 vStartCPTangent = ComputeTangent( iStartCP );
	const glm::vec3 vEndCPTangent = ComputeTangent( iEndCP );

	return fStartCPFactor * m_aControlPoints[ iStartCP ] + fStartCPTangentFactor * vStartCPTangent + fEndCPFactor * m_aControlPoints[ iEndCP ] + fEndCPTangentFactor * vEndCPTangent;
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
		m_xSpline->m_aControlPoints[ m_iCPIndex ] = GetEntity()->GetPosition();
	else
		GetEntity()->SetPosition( m_xSpline->m_aControlPoints[ m_iCPIndex ] );
	

	DisplayControlPointVisual( GetEntity(), glm::vec3( 0.f, 0.f, 1.f ) );
}

void SplineCPComponent::SetSplineData( SplineComponent* pSpline, const int iCPIndex )
{
	m_xSpline = pSpline;
	m_iCPIndex = iCPIndex;
}
