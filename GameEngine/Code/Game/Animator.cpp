#include "Animator.h"

#include "Entity.h"
#include "GameEngine.h"
#include "Graphics/Visual.h"
#include "Math/GLMHelpers.h"

REGISTER_COMPONENT( AnimatorComponent );
SET_COMPONENT_PRIORITY_AFTER( AnimatorComponent, VisualComponent );

AnimatorComponent::AnimatorComponent( Entity* pEntity )
	: Component( pEntity )
	, m_fRunningTime( 0.f )
	, m_uAnimationIndex( 0 )
	, m_eAnimationState( AnimationState::STOPPED )
	, m_eAnimationType( AnimationType::LOOP )
{
}

void AnimatorComponent::Setup( const char* sModelFile )
{
	m_sModelFile = sModelFile;
}

void AnimatorComponent::Initialize()
{
	m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
}

bool AnimatorComponent::IsInitialized() const
{
	return m_xModel->IsLoading() == false;
}

void AnimatorComponent::Start()
{
	m_fRunningTime = 0.f;
	m_aBoneMatrices.Resize( m_xModel->GetSkinMatrices().Count(), glm::mat4( 1.f ) );

	PlayAnimation();
}

void AnimatorComponent::Stop()
{
	StopAnimation();

	m_fRunningTime = 0.f;
	m_aBoneMatrices.Clear();
}

void AnimatorComponent::Update( const GameContext& oGameContext )
{
	if( m_xModel->GetAnimations().Empty() )
		return;

	const Skeleton& oSkeleton = m_xModel->GetSkeleton();
	const Animation& oAnimation = m_xModel->GetAnimations()[ m_uAnimationIndex ];

	if( m_eAnimationState == AnimationState::PLAYING )
	{
		m_fRunningTime += oGameContext.m_fLastDeltaTime;

		switch( m_eAnimationType )
		{
		case AnimationType::ONCE:
			if( m_fRunningTime >= oAnimation.m_fDuration )
				StopAnimation();
			break;
		case AnimationType::STAY:
			if( m_fRunningTime >= oAnimation.m_fDuration )
				PauseAnimation();
			break;
		case AnimationType::LOOP:
			while( m_fRunningTime >= oAnimation.m_fDuration )
				m_fRunningTime -= oAnimation.m_fDuration;
			break;
		}
	}

	if( m_eAnimationState != AnimationState::STOPPED )
	{
		for( uint u = 0; u < m_aBoneMatrices.Count(); ++u )
			m_aBoneMatrices[ u ] = m_xModel->GetPoseMatrices()[ u ];

		for( const NodeAnimation& oNodeAnimation : oAnimation.m_aNodeAnimations )
		{
			Transform oTransform;
			oTransform.SetPosition( oNodeAnimation.m_oPositionCurve.Evaluate( m_fRunningTime ) );
			oTransform.SetRotation( oNodeAnimation.m_oRotationCurve.Evaluate( m_fRunningTime ) );
			oTransform.SetScale( oNodeAnimation.m_oScaleCurve.Evaluate( m_fRunningTime ) );
			m_aBoneMatrices[ oNodeAnimation.m_uMatrixIndex ] = oTransform.GetMatrixTRS();
		}

		oSkeleton.Update( glm::mat4( 1.f ), m_aBoneMatrices );

		for( uint u = 0; u < m_aBoneMatrices.Count(); ++u )
			m_aBoneMatrices[ u ] = m_aBoneMatrices[ u ] * m_xModel->GetSkinMatrices()[ u ];
	}
	else
	{
		for( uint u = 0; u < m_aBoneMatrices.Count(); ++u )
			m_aBoneMatrices[ u ] = glm::mat4( 1.f );
	}

	const Array< VisualNode* > aNodes = g_pRenderer->m_oVisualStructure.FindNodes( GetEntity() );
	for( VisualNode* pNode : aNodes )
		pNode->m_aBoneMatrices = m_aBoneMatrices;
}

void AnimatorComponent::Dispose()
{
	m_xModel = nullptr;
}

void AnimatorComponent::DisplayInspector()
{
	auto GetAnimationTypeName = []( const AnimationType eAnimationType ) {
		switch( eAnimationType )
		{
		case AnimationType::ONCE:
			return "ONCE";
		case AnimationType::STAY:
			return "STAY";
		case AnimationType::LOOP:
			return "LOOP";
		}

		return "UNKNOWN";
	};

	auto GetAnimationStateName = []( const AnimationState eAnimationState ) {
		switch( eAnimationState )
		{
		case AnimationState::PLAYING:
			return "PLAYING";
		case AnimationState::STOPPED:
			return "STOPPED";
		case AnimationState::PAUSED:
			return "PAUSED";
		}

		return "UNKNOWN";
	};

	if( ImGui::CollapsingHeader( "Animation" ) )
	{
		if( m_xModel->GetAnimations().Empty() )
		{
			ImGui::Text( "No animation available" );
			return;
		}

		if( ImGui::BeginCombo( "Selected animation", m_xModel->GetAnimations()[ m_uAnimationIndex ].m_sName.c_str() ) )
		{
			for( uint u = 0; u < m_xModel->GetAnimations().Count(); ++u )
			{
				const Animation& oAnimation = m_xModel->GetAnimations()[ u ];
				if( ImGui::Selectable( oAnimation.m_sName.c_str(), u == m_uAnimationIndex ) )
					m_uAnimationIndex = u;
			}
			ImGui::EndCombo();
		}

		if( ImGui::BeginCombo( "Animation type", GetAnimationTypeName( m_eAnimationType ) ) )
		{
			for( uint u = 0; u < ( uint )AnimationType::_COUNT; ++u )
			{
				AnimationType eAnimationType = AnimationType( u );
				if( ImGui::Selectable( GetAnimationTypeName( eAnimationType ), eAnimationType == m_eAnimationType ) )
					m_eAnimationType = eAnimationType;
			}
			ImGui::EndCombo();
		}

		const float fDuration = m_xModel->GetAnimations()[ m_uAnimationIndex ].m_fDuration;
		ImGui::SliderFloat( "Time", &m_fRunningTime, 0.f, fDuration );

		ImGui::Text( std::format( "Animation state : {}", GetAnimationStateName( m_eAnimationState ) ).c_str() );

		if( ImGui::Button( "Play" ) )
			PlayAnimation();
		ImGui::SameLine();
		if( ImGui::Button( "Stop" ) )
			StopAnimation();
		ImGui::SameLine();
		if( m_eAnimationState == AnimationState::PLAYING )
		{
			if( ImGui::Button( "Pause" ) )
				PauseAnimation();
		}
		else
		{
			if( ImGui::Button( "Resume" ) )
				ResumeAnimation();
		}
	}
}

void AnimatorComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Model" )
	{
		m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
		m_aBoneMatrices.Resize( m_xModel->GetSkinMatrices().Count(), glm::mat4( 1.f ) );
	}
}

void AnimatorComponent::PlayAnimation()
{
	m_fRunningTime = 0.f;
	m_eAnimationState = AnimationState::PLAYING;
}

void AnimatorComponent::StopAnimation()
{
	m_fRunningTime = 0.f;
	m_eAnimationState = AnimationState::STOPPED;
}

void AnimatorComponent::ResumeAnimation()
{
	m_eAnimationState = AnimationState::PLAYING;
}

void AnimatorComponent::PauseAnimation()
{
	m_eAnimationState = AnimationState::PAUSED;
}

const Array< glm::mat4x3 >& AnimatorComponent::GetBoneMatrices() const
{
	return m_aBoneMatrices;
}
