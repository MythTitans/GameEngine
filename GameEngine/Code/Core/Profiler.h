#pragma once

#include <mutex>

#include "Array.h"
#include "ImGui/imgui.h"

inline constexpr uint GPU_QUERY_COUNT = 1024;

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

class GPUProfilerBlock
{
public:
	friend class Profiler;

	explicit GPUProfilerBlock( const char* sName );
	~GPUProfilerBlock();

private:
	uint m_uBlockID;
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

	uint	StartGPUBlock( const char* sName );
	void	EndGPUBlock( const uint uBlockID );

	uint	StartAsyncBlock( const char* sName );
	void	EndAsyncBlock( const uint uBlockID );

private:
	void	DrawGrid( const float fReferenceWidth );
	ImVec2	DrawBlock( const char* sName, const char* sTooltip, const float fStart, const float fEnd, const int iDepth );

	Array< Frame >				m_aFrames;
	Array< Frame* >				m_aPendingFrames;
	uint						m_uCurrentFrameIndex;

	uint						m_uBlocksDepth;
	uint						m_uAsyncBlocksDepth;
	uint						m_uAsyncBlocksCount;
	uint						m_uGPUBlocksDepth;

	bool						m_bDisplayProfiler;
	bool						m_bPauseProfiler;

	std::mutex					m_oFrameMutex;

	uint						m_aGPUQueries[ GPU_QUERY_COUNT ];
	Array< uint >				m_aAvailableGPUQueries;
};

extern Profiler* g_pProfiler;