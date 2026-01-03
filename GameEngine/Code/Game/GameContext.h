#pragma once

#include "Core/Time.h"
#include "Core/Types.h"

struct GameContext
{
	GameContext();

	GameTimePoint	m_oFrameStart;
	uint64			m_uFrameIndex;

	float			m_fLastDeltaTime;
	float			m_fLastRealDeltaTime;

	uint			m_uLastTicks;

	bool			m_bEditing;
};
