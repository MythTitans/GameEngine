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
	bool						IsInitialized() override;
	void						Start() override;
	void						Stop() override;
	void						Update( const float fDeltaTime ) override;

	void						PlayAnimation();
	void						StopAnimation();
	void						ResumeAnimation();
	void						PauseAnimation();

	const Array< glm::mat4 >&	GetBoneMatrices() const;

	void						DisplayInspector();

private:
	std::string			m_sModelFile;
	ModelResPtr			m_xModel;
	Array< glm::mat4 >	m_aBoneMatrices;
	uint				m_uAnimationIndex;

	float				m_fRunningTime;
	AnimationState		m_eAnimationState;
	AnimationType		m_eAnimationType;
};
