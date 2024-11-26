#pragma once

#include <mutex>
#include <chrono>

#include "Array.h"
#include "ImGui/imgui.h"

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

	void NewFrame();
	void Display();
		 
	uint StartBlock( const char* sName );
	void EndBlock( const uint uBlockID );

	uint StartAsyncBlock( const char* sName );
	void EndAsyncBlock( const uint uBlockID );

private:
	using TimePoint = std::chrono::high_resolution_clock::time_point;

	struct Block
	{
		Block( const char* sName, const uint uDepth )
			: m_sName( sName )
			, m_oStart( std::chrono::high_resolution_clock::now() )
			, m_uDepth( uDepth )
		{
		}

		bool IsFinished() const
		{
			return m_oStart <= m_oEnd;
		}

		const char*	m_sName;
		TimePoint	m_oStart;
		TimePoint	m_oEnd;
		uint		m_uDepth;
	};

	struct Frame
	{
		TimePoint					m_oFrameStart;
		Array< Block, FAST_RESIZE >	m_aBlocks;
		Array< Block, FAST_RESIZE > m_aAsyncBlocks;
	};

	void	DrawGrid( const float fReferenceWidth );
	ImVec2	DrawBlock( const char* sName, const char* sTooltip, const float fStart, const float fEnd, const int iDepth );

	Frame						m_oCurrentFrame;
	Frame						m_oPreviousFrame;

	//Array< Block, FAST_RESIZE >	m_aBlocks;
	uint						m_uBlocksDepth;
	uint						m_uAsyncBlocksDepth;
	uint						m_uAsyncBlocksInFlight;

	bool						m_bDisplayProfiler;

	std::mutex					m_oFrameMutex;

	//TimePoint					m_oFrameStart;
};

extern Profiler* g_pProfiler;