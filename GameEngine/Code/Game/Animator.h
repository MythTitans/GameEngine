#pragma once

#include "Component.h"
#include "ResourceLoader.h"

enum class AnimationState : uint8
{
	PLAYING,
	STOPPED,
	PAUSED,
};

enum class AnimationType : uint8
{
	ONCE,
	STAY,
	LOOP,
	_COUNT
};

class AnimatorComponent : public Component
{
public:
	explicit AnimatorComponent( Entity* pEntity );

	void						Setup( const char* sModelFile );
	void						Initialize() override;
	bool						IsInitialized() const override;
	void						Start() override;
	void						Stop() override;
	void						Update( const GameContext& oGameContext ) override;
	void						Dispose() override;

	void						DisplayInspector() override;
	void						OnPropertyChanged( const std::string& sProperty ) override;

	void						PlayAnimation();
	void						StopAnimation();
	void						ResumeAnimation();
	void						PauseAnimation();

	const Array< glm::mat4x3 >&	GetBoneMatrices() const;

private:
	PROPERTIES( AnimatorComponent );
	PROPERTY( "Model", m_sModelFile, std::string );

	ModelResPtr				m_xModel;
	Array< glm::mat4x3 >	m_aBoneMatrices;
	uint					m_uAnimationIndex;

	float					m_fRunningTime;
	AnimationState			m_eAnimationState;
	AnimationType			m_eAnimationType;
};
