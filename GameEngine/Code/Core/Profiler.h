#pragma once

#include <mutex>

#include "Array.h"
#include "ImGui/imgui.h"

struct Frame;

class ProfilerBlock
{
public:
	explicit ProfilerBlock( const char* sName, const bool bAsync = false );
	~ProfilerBlock();

private:
	uint m_uBlockID;
	bool m_bAsync;
};

class Profiler
{
public:
	Profiler();
	~Profiler();

	void	NewFrame();
	void	Display();
			
	uint	StartBlock( const char* sName );
	void	EndBlock( const uint uBlockID );

	uint	StartAsyncBlock( const char* sName );
	void	EndAsyncBlock( const uint uBlockID );

private:
	void	DrawGrid( const float fReferenceWidth );
	ImVec2	DrawBlock( const char* sName, const char* sTooltip, const float fStart, const float fEnd, const int iDepth );

	Array< Frame >				m_aFrames;
	uint						m_uCurrentFrameIndex;

	uint						m_uBlocksDepth;
	uint						m_uAsyncBlocksDepth;
	uint						m_uAsyncBlocksCount;

	bool						m_bDisplayProfiler;
	bool						m_bPauseProfiler;

	std::mutex					m_oFrameMutex;
};

extern Profiler* g_pProfiler;