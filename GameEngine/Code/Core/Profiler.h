#pragma once

#include <chrono>

#include "Array.h"
#include "ImGui/imgui.h"

class ProfilerBlock
{
public:
	explicit ProfilerBlock( const char* sName );
	~ProfilerBlock();

private:
	uint m_uBlockID;
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

private:
	using TimePoint = std::chrono::high_resolution_clock::time_point;

	struct Block
	{
		Block( const char* sName, const Profiler& oProfiler )
			: m_sName( sName )
			, m_oStart( std::chrono::high_resolution_clock::now() )
			, m_uDepth( oProfiler.m_uBlocksDepth )
		{
		}

		const char*	m_sName;
		TimePoint	m_oStart;
		TimePoint	m_oEnd;
		uint		m_uDepth;
	};

	void	DrawGrid( const float fReferenceWidth );
	void	DrawBlock( const char* sName, const ImColor& oColor, const float fStart, const float fEnd, const int iDepth );

	Array< Block, FAST_RESIZE >	m_aBlocks;
	uint						m_uBlocksDepth;

	bool						m_bDisplayProfiler;

	TimePoint					m_oFrameStart;
};

extern Profiler* g_pProfiler;