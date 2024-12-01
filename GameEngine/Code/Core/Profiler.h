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

		const char*		m_sName;
		GameTimePoint	m_oStart;
		GameTimePoint	m_oEnd;
		uint			m_uDepth;
	};

	struct AsyncBlock : Block
	{
		AsyncBlock( const uint uID, const char* sName, const uint uDepth )
			: Block( sName, uDepth )
			, m_uID( uID )
		{
		}

		uint m_uID;
	};

	struct Frame
	{
		GameTimePoint						m_oFrameStart;
		GameTimePoint						m_oFrameEnd;
		Array< Block, FAST_RESIZE >			m_aBlocks;
		Array< AsyncBlock, FAST_RESIZE >	m_aAsyncBlocks;
	};

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