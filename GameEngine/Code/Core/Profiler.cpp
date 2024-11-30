#include "Profiler.h"

#include <format>

#include "Game/InputHandler.h"

static const uint FRAME_HISTORY_COUNT = 61;

static constexpr ImColor BorderColor( const ImColor& oBackgroundColor )
{
	return ImColor( max( 0.f, oBackgroundColor.Value.x * 0.8f ), max( 0.f, oBackgroundColor.Value.y * 0.8f ), max( 0.f, oBackgroundColor.Value.z * 0.8f ), 1.f );
}

static constexpr ImColor TextColor( const ImColor& oBackgroundColor )
{
	const float fLum = 0.299f * oBackgroundColor.Value.x + 0.587f * oBackgroundColor.Value.y + 0.114f * oBackgroundColor.Value.z;
	return fLum > 0.5f ? ImColor( 0.05f, 0.05f, 0.05f ) : ImColor( 0.95f, 0.95f, 0.95f );
}

static constexpr ImColor BACKGROUND_COLORS[ 64 ] = {
	ImColor( 255, 0, 0 ),		ImColor( 0, 255, 0 ),		ImColor( 0, 0, 255 ),		ImColor( 255, 255, 0 ),
	ImColor( 0, 255, 255 ),		ImColor( 255, 0, 255 ),		ImColor( 128, 0, 0 ),		ImColor( 0, 128, 0 ),
	ImColor( 0, 0, 128 ),		ImColor( 128, 128, 0 ),		ImColor( 0, 128, 128 ),		ImColor( 128, 0, 128 ),
	ImColor( 255, 128, 0 ),		ImColor( 128, 255, 0 ),		ImColor( 0, 255, 128 ),		ImColor( 0, 128, 255 ),
	ImColor( 128, 0, 255 ),		ImColor( 255, 0, 128 ),		ImColor( 192, 64, 0 ),		ImColor( 64, 192, 0 ),
	ImColor( 0, 192, 64 ),		ImColor( 0, 64, 192 ),		ImColor( 64, 0, 192 ),		ImColor( 192, 0, 64 ),
	ImColor( 192, 192, 64 ),	ImColor( 192, 64, 192 ),	ImColor( 64, 192, 192 ),	ImColor( 64, 128, 64 ),
	ImColor( 128, 64, 64 ),		ImColor( 64, 64, 128 ),		ImColor( 128, 128, 64 ),	ImColor( 128, 64, 128 ),
	ImColor( 64, 128, 128 ),	ImColor( 255, 64, 64 ),		ImColor( 64, 255, 64 ),		ImColor( 64, 64, 255 ),
	ImColor( 255, 64, 192 ),	ImColor( 192, 255, 64 ),	ImColor( 64, 192, 255 ),	ImColor( 255, 192, 64 ),
	ImColor( 64, 255, 192 ),	ImColor( 192, 64, 255 ),	ImColor( 255, 128, 64 ),	ImColor( 128, 255, 64 ),
	ImColor( 64, 128, 255 ),	ImColor( 255, 64, 128 ),	ImColor( 128, 64, 255 ),	ImColor( 64, 255, 128 ),
	ImColor( 192, 128, 64 ),	ImColor( 128, 192, 64 ),	ImColor( 64, 192, 128 ),	ImColor( 64, 128, 192 ),
	ImColor( 128, 64, 192 ),	ImColor( 192, 64, 128 ),	ImColor( 192, 128, 128 ),	ImColor( 128, 192, 192 ),
	ImColor( 192, 192, 128 ),	ImColor( 128, 128, 192 ),	ImColor( 192, 128, 192 ),	ImColor( 128, 192, 128 ),
	ImColor( 255, 128, 128 ),	ImColor( 128, 255, 128 ),	ImColor( 128, 128, 255 ),	ImColor( 255, 64, 6 )
};

static constexpr ImColor BORDER_COLORS[ 64 ] = {
	BorderColor( BACKGROUND_COLORS[ 0 ] ),	BorderColor( BACKGROUND_COLORS[ 1 ] ),	BorderColor( BACKGROUND_COLORS[ 2 ] ),	BorderColor( BACKGROUND_COLORS[ 3 ] ),
	BorderColor( BACKGROUND_COLORS[ 4 ] ),	BorderColor( BACKGROUND_COLORS[ 5 ] ),	BorderColor( BACKGROUND_COLORS[ 6 ] ),	BorderColor( BACKGROUND_COLORS[ 7 ] ),
	BorderColor( BACKGROUND_COLORS[ 8 ] ),	BorderColor( BACKGROUND_COLORS[ 9 ] ),	BorderColor( BACKGROUND_COLORS[ 10 ] ),	BorderColor( BACKGROUND_COLORS[ 11 ] ),
	BorderColor( BACKGROUND_COLORS[ 12 ] ), BorderColor( BACKGROUND_COLORS[ 13 ] ),	BorderColor( BACKGROUND_COLORS[ 14 ] ),	BorderColor( BACKGROUND_COLORS[ 15 ] ),
	BorderColor( BACKGROUND_COLORS[ 16 ] ), BorderColor( BACKGROUND_COLORS[ 17 ] ),	BorderColor( BACKGROUND_COLORS[ 18 ] ),	BorderColor( BACKGROUND_COLORS[ 19 ] ),
	BorderColor( BACKGROUND_COLORS[ 20 ] ), BorderColor( BACKGROUND_COLORS[ 21 ] ),	BorderColor( BACKGROUND_COLORS[ 22 ] ),	BorderColor( BACKGROUND_COLORS[ 23 ] ),
	BorderColor( BACKGROUND_COLORS[ 24 ] ), BorderColor( BACKGROUND_COLORS[ 25 ] ),	BorderColor( BACKGROUND_COLORS[ 26 ] ),	BorderColor( BACKGROUND_COLORS[ 27 ] ),
	BorderColor( BACKGROUND_COLORS[ 28 ] ), BorderColor( BACKGROUND_COLORS[ 29 ] ),	BorderColor( BACKGROUND_COLORS[ 30 ] ),	BorderColor( BACKGROUND_COLORS[ 31 ] ),
	BorderColor( BACKGROUND_COLORS[ 32 ] ), BorderColor( BACKGROUND_COLORS[ 33 ] ),	BorderColor( BACKGROUND_COLORS[ 34 ] ),	BorderColor( BACKGROUND_COLORS[ 35 ] ),
	BorderColor( BACKGROUND_COLORS[ 36 ] ), BorderColor( BACKGROUND_COLORS[ 37 ] ),	BorderColor( BACKGROUND_COLORS[ 38 ] ),	BorderColor( BACKGROUND_COLORS[ 39 ] ),
	BorderColor( BACKGROUND_COLORS[ 40 ] ), BorderColor( BACKGROUND_COLORS[ 41 ] ),	BorderColor( BACKGROUND_COLORS[ 42 ] ),	BorderColor( BACKGROUND_COLORS[ 43 ] ),
	BorderColor( BACKGROUND_COLORS[ 44 ] ), BorderColor( BACKGROUND_COLORS[ 45 ] ),	BorderColor( BACKGROUND_COLORS[ 46 ] ),	BorderColor( BACKGROUND_COLORS[ 47 ] ),
	BorderColor( BACKGROUND_COLORS[ 48 ] ), BorderColor( BACKGROUND_COLORS[ 49 ] ),	BorderColor( BACKGROUND_COLORS[ 50 ] ),	BorderColor( BACKGROUND_COLORS[ 51 ] ),
	BorderColor( BACKGROUND_COLORS[ 52 ] ), BorderColor( BACKGROUND_COLORS[ 53 ] ),	BorderColor( BACKGROUND_COLORS[ 54 ] ),	BorderColor( BACKGROUND_COLORS[ 55 ] ),
	BorderColor( BACKGROUND_COLORS[ 56 ] ), BorderColor( BACKGROUND_COLORS[ 57 ] ),	BorderColor( BACKGROUND_COLORS[ 58 ] ),	BorderColor( BACKGROUND_COLORS[ 59 ] ),
	BorderColor( BACKGROUND_COLORS[ 60 ] ), BorderColor( BACKGROUND_COLORS[ 61 ] ),	BorderColor( BACKGROUND_COLORS[ 62 ] ),	BorderColor( BACKGROUND_COLORS[ 63 ] )
};

static constexpr ImColor TEXT_COLORS[ 64 ] = {
	TextColor( BACKGROUND_COLORS[ 0 ] ),	TextColor( BACKGROUND_COLORS[ 1 ] ),	TextColor( BACKGROUND_COLORS[ 2 ] ),	TextColor( BACKGROUND_COLORS[ 3 ] ),
	TextColor( BACKGROUND_COLORS[ 4 ] ),	TextColor( BACKGROUND_COLORS[ 5 ] ),	TextColor( BACKGROUND_COLORS[ 6 ] ),	TextColor( BACKGROUND_COLORS[ 7 ] ),
	TextColor( BACKGROUND_COLORS[ 8 ] ),	TextColor( BACKGROUND_COLORS[ 9 ] ),	TextColor( BACKGROUND_COLORS[ 10 ] ),	TextColor( BACKGROUND_COLORS[ 11 ] ),
	TextColor( BACKGROUND_COLORS[ 12 ] ),	TextColor( BACKGROUND_COLORS[ 13 ] ),	TextColor( BACKGROUND_COLORS[ 14 ] ),	TextColor( BACKGROUND_COLORS[ 15 ] ),
	TextColor( BACKGROUND_COLORS[ 16 ] ),	TextColor( BACKGROUND_COLORS[ 17 ] ),	TextColor( BACKGROUND_COLORS[ 18 ] ),	TextColor( BACKGROUND_COLORS[ 19 ] ),
	TextColor( BACKGROUND_COLORS[ 20 ] ),	TextColor( BACKGROUND_COLORS[ 21 ] ),	TextColor( BACKGROUND_COLORS[ 22 ] ),	TextColor( BACKGROUND_COLORS[ 23 ] ),
	TextColor( BACKGROUND_COLORS[ 24 ] ),	TextColor( BACKGROUND_COLORS[ 25 ] ),	TextColor( BACKGROUND_COLORS[ 26 ] ),	TextColor( BACKGROUND_COLORS[ 27 ] ),
	TextColor( BACKGROUND_COLORS[ 28 ] ),	TextColor( BACKGROUND_COLORS[ 29 ] ),	TextColor( BACKGROUND_COLORS[ 30 ] ),	TextColor( BACKGROUND_COLORS[ 31 ] ),
	TextColor( BACKGROUND_COLORS[ 32 ] ),	TextColor( BACKGROUND_COLORS[ 33 ] ),	TextColor( BACKGROUND_COLORS[ 34 ] ),	TextColor( BACKGROUND_COLORS[ 35 ] ),
	TextColor( BACKGROUND_COLORS[ 36 ] ),	TextColor( BACKGROUND_COLORS[ 37 ] ),	TextColor( BACKGROUND_COLORS[ 38 ] ),	TextColor( BACKGROUND_COLORS[ 39 ] ),
	TextColor( BACKGROUND_COLORS[ 40 ] ),	TextColor( BACKGROUND_COLORS[ 41 ] ),	TextColor( BACKGROUND_COLORS[ 42 ] ),	TextColor( BACKGROUND_COLORS[ 43 ] ),
	TextColor( BACKGROUND_COLORS[ 44 ] ),	TextColor( BACKGROUND_COLORS[ 45 ] ),	TextColor( BACKGROUND_COLORS[ 46 ] ),	TextColor( BACKGROUND_COLORS[ 47 ] ),
	TextColor( BACKGROUND_COLORS[ 48 ] ),	TextColor( BACKGROUND_COLORS[ 49 ] ),	TextColor( BACKGROUND_COLORS[ 50 ] ),	TextColor( BACKGROUND_COLORS[ 51 ] ),
	TextColor( BACKGROUND_COLORS[ 52 ] ),	TextColor( BACKGROUND_COLORS[ 53 ] ),	TextColor( BACKGROUND_COLORS[ 54 ] ),	TextColor( BACKGROUND_COLORS[ 55 ] ),
	TextColor( BACKGROUND_COLORS[ 56 ] ),	TextColor( BACKGROUND_COLORS[ 57 ] ),	TextColor( BACKGROUND_COLORS[ 58 ] ),	TextColor( BACKGROUND_COLORS[ 59 ] ),
	TextColor( BACKGROUND_COLORS[ 60 ] ),	TextColor( BACKGROUND_COLORS[ 61 ] ),	TextColor( BACKGROUND_COLORS[ 62 ] ),	TextColor( BACKGROUND_COLORS[ 63 ] )
};

static uint GenerateColorIndex( const char* sName )
{
	uint uHash = 0;
	while( *sName )
	{
		uHash = ( uHash * 31 ) ^ uint( *sName );
		++sName;
	}

	return uHash % 64;
}

ProfilerBlock::ProfilerBlock( const char* sName, const bool bAsync )
	: m_uBlockID( bAsync ? g_pProfiler->StartAsyncBlock( sName ) : g_pProfiler->StartBlock( sName ) )
	, m_bAsync( bAsync )
{
}

ProfilerBlock::~ProfilerBlock()
{
	m_bAsync ? g_pProfiler->EndAsyncBlock( m_uBlockID ) : g_pProfiler->EndBlock( m_uBlockID );
}

Profiler* g_pProfiler = nullptr;

Profiler::Profiler()
	: m_aFrames( FRAME_HISTORY_COUNT )
	, m_uCurrentFrameIndex( 0 )
	, m_uBlocksDepth( 0 )
	, m_uAsyncBlocksDepth( 0 )
	, m_uAsyncBlocksCount( 0 )
	, m_bDisplayProfiler( false )
	, m_bPauseProfiler( false )
{
	g_pProfiler = this;
}

Profiler::~Profiler()
{
	g_pProfiler = nullptr;
}

void Profiler::NewFrame()
{
	std::unique_lock oLock( m_oFrameMutex );

 	Frame& oPreviousFrame = m_aFrames[ m_uCurrentFrameIndex ];

	if( m_bPauseProfiler == false)
 		m_uCurrentFrameIndex = ( m_uCurrentFrameIndex + 1 ) % FRAME_HISTORY_COUNT;
	
 	Frame& oCurrentFrame = m_aFrames[ m_uCurrentFrameIndex ];

	oCurrentFrame.m_aBlocks.Clear();
	oCurrentFrame.m_aAsyncBlocks.Clear();

	oCurrentFrame.m_oFrameStart = std::chrono::high_resolution_clock::now();
	oPreviousFrame.m_oFrameEnd = oCurrentFrame.m_oFrameStart;

	int uAsyncBlocksInFlight = 0;

	for( uint u = 0; u < oPreviousFrame.m_aAsyncBlocks.Count(); ++u )
	{
		if( oPreviousFrame.m_aAsyncBlocks[ u ].IsFinished() == false )
		{
			if( uAsyncBlocksInFlight == 0 )
				uAsyncBlocksInFlight = u;

			oCurrentFrame.m_aAsyncBlocks.PushBack( oPreviousFrame.m_aAsyncBlocks[ u ] );
			oCurrentFrame.m_aAsyncBlocks.Back().m_oStart = oCurrentFrame.m_oFrameStart;
			oPreviousFrame.m_aAsyncBlocks[ u ].m_oEnd = oCurrentFrame.m_oFrameStart;
		}
	}
}

void Profiler::Display()
{
	ProfilerBlock oBlock( "Profiler" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_PROFILER ) )
		m_bDisplayProfiler = !m_bDisplayProfiler;

	if( m_bDisplayProfiler )
	{
		ImGui::Begin( "Profiler" );

		ImGui::Checkbox( "Pause", &m_bPauseProfiler );

		float fMinFrameLength = FLT_MAX;
		float fMaxFrameLength = 0.f;
		float fAvgFrameLength = 0.f;
		float aFrameLengths[ FRAME_HISTORY_COUNT - 1 ];
		for( uint u = 0; u < FRAME_HISTORY_COUNT - 1; ++u )
		{
			int iFrameIndex = ( int )m_uCurrentFrameIndex - ( int )( u + 1 );
			if( iFrameIndex < 0 )
				iFrameIndex += FRAME_HISTORY_COUNT;

			const uint64 uLengthMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( m_aFrames[ iFrameIndex ].m_oFrameEnd - m_aFrames[ iFrameIndex ].m_oFrameStart ).count();
			const float fLengthMilliSeconds = uLengthMicroSeconds / 1000.f;

			if( fLengthMilliSeconds < fMinFrameLength )
				fMinFrameLength = fLengthMilliSeconds;

			if( fMaxFrameLength < fLengthMilliSeconds )
				fMaxFrameLength = fLengthMilliSeconds;

			fAvgFrameLength += fLengthMilliSeconds;

			aFrameLengths[ FRAME_HISTORY_COUNT - 2 - u ] = fLengthMilliSeconds;
		}

		fAvgFrameLength /= FRAME_HISTORY_COUNT;

		const float fHistogramMin = min( fAvgFrameLength, fMinFrameLength ) * 0.8f;
		const float fHistogramMax = max( fAvgFrameLength, fMaxFrameLength ) * 1.2f;

		ImGui::PlotHistogram( "Frame history", aFrameLengths, IM_ARRAYSIZE( aFrameLengths ), 0, NULL, fHistogramMin, fHistogramMax, ImVec2( 0, 80.0f ) );

		static int iSlider = 0;
		if( m_bPauseProfiler == false )
			iSlider = 0;
		ImGui::SliderInt( "Frame index", &iSlider, -( int )FRAME_HISTORY_COUNT + 2, 0 );

		static float fZoom = 1.f;
		ImGui::SliderFloat( "Zoom", &fZoom, 0.1f, 10.f, "%.1f" );

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.2f, 0.2f, 0.2f, 1.f ) );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.f, 0.f ) );
		ImGui::BeginChild( "Profiler", ImVec2( 0.f, 0.f ), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar );
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		const int iBucketBaseSizePx = 100;
		const float fReferenceWidth = iBucketBaseSizePx * fZoom;
		DrawGrid( fReferenceWidth );

		int iDisplayedFrameIndex = ( int )m_uCurrentFrameIndex + iSlider - 1;
		if( iDisplayedFrameIndex < 0 )
			iDisplayedFrameIndex += FRAME_HISTORY_COUNT;

		Frame& oDisplayedFrame = m_aFrames[ iDisplayedFrameIndex ];

		ImGui::Text( "\n Main thread" );

		float fMaxX = 0.f;
		float fMaxY = 0.f;
		for( const Block& oBlock : oDisplayedFrame.m_aBlocks )
		{
			const uint64 uStartMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oStart - oDisplayedFrame.m_oFrameStart ).count();
			const float fStartMilliSeconds = uStartMicroSeconds / 1000.f;

			const uint64 uEndMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oEnd - oDisplayedFrame.m_oFrameStart ).count();
			const float fEndMilliSeconds = uEndMicroSeconds / 1000.f;

			const float fStart = fStartMilliSeconds * fReferenceWidth;
			const float fEnd = fEndMilliSeconds * fReferenceWidth;

			const ImVec2 vCursorPos = DrawBlock( oBlock.m_sName, std::format( "{} ({:.3f} ms)", oBlock.m_sName, fEndMilliSeconds - fStartMilliSeconds ).c_str(), fStart, fEnd, oBlock.m_uDepth );

			if( fMaxX < vCursorPos.x )
				fMaxX = vCursorPos.x;
			if( fMaxY < vCursorPos.y )
				fMaxY = vCursorPos.y;
		}

		ImGui::SetCursorScreenPos( ImVec2( ImGui::GetCursorScreenPos().x, fMaxY ) );
		ImGui::Separator();

		ImGui::Text( "\n IO thread" );

		for( const Block& oBlock : oDisplayedFrame.m_aAsyncBlocks )
		{
			const uint64 uStartMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oStart - oDisplayedFrame.m_oFrameStart ).count();
			const float fStartMilliSeconds = uStartMicroSeconds / 1000.f;

			const uint64 uEndMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oEnd - oDisplayedFrame.m_oFrameStart ).count();
			const float fEndMilliSeconds = uEndMicroSeconds / 1000.f;

			const float fStart = fStartMilliSeconds * fReferenceWidth;
			const float fEnd = fEndMilliSeconds * fReferenceWidth;

			const ImVec2 vCursorPos = DrawBlock( oBlock.m_sName, std::format( "{} ({:.3f} ms)", oBlock.m_sName, fEndMilliSeconds - fStartMilliSeconds ).c_str(), fStart, fEnd, oBlock.m_uDepth );

			if( fMaxX < vCursorPos.x )
				fMaxX = vCursorPos.x;
			if( fMaxY < vCursorPos.y )
				fMaxY = vCursorPos.y;
		}

		ImGui::SetCursorScreenPos( ImVec2( ImGui::GetCursorScreenPos().x, fMaxY ) );
		ImGui::Separator();

		ImGui::SetCursorScreenPos( ImVec2( fMaxX, ImGui::GetCursorScreenPos().y ) );

		ImGui::EndChild();

		ImGui::End();
	}
}

uint Profiler::StartBlock( const char* sName )
{
	const uint uID = m_aFrames[ m_uCurrentFrameIndex ].m_aBlocks.Count();
	m_aFrames[ m_uCurrentFrameIndex ].m_aBlocks.PushBack( Block( sName, m_uBlocksDepth ) );
	++m_uBlocksDepth;
	return uID;
}

void Profiler::EndBlock( const uint uBlockID )
{
	m_aFrames[ m_uCurrentFrameIndex ].m_aBlocks[ uBlockID ].m_oEnd = std::chrono::high_resolution_clock::now();
	--m_uBlocksDepth;
}

uint Profiler::StartAsyncBlock( const char* sName )
{
	std::unique_lock oLock( m_oFrameMutex );

	const uint uID = m_uAsyncBlocksCount;
	m_aFrames[ m_uCurrentFrameIndex ].m_aAsyncBlocks.PushBack( AsyncBlock( uID, sName, m_uAsyncBlocksDepth ) );
	++m_uAsyncBlocksDepth;
	++m_uAsyncBlocksCount;
	return uID;
}

void Profiler::EndAsyncBlock( const uint uBlockID )
{
	std::unique_lock oLock( m_oFrameMutex );

	for( AsyncBlock& oBlock : m_aFrames[ m_uCurrentFrameIndex ].m_aAsyncBlocks )
	{
		if( oBlock.m_uID == uBlockID )
		{
			oBlock.m_oEnd = std::chrono::high_resolution_clock::now();
			break;
		}
	}

	--m_uAsyncBlocksDepth;
}

void Profiler::DrawGrid( const float fReferenceWidth )
{
	const ImVec2 oSize = ImVec2( ImGui::GetContentRegionAvail().x + ImGui::GetScrollMaxX(), ImGui::GetContentRegionAvail().y ) ;
	const float fBucketWidth = std::floorf( fReferenceWidth );
	const int iLines = ( int )( oSize.x / fBucketWidth );
	for( int i = 1; i <= iLines; ++i )
	{
		const ImVec2 vFrom = ImGui::GetCursorScreenPos();
		if( i % 5 == 0 )
			ImGui::GetWindowDrawList()->AddLine( ImVec2( vFrom.x + i * fBucketWidth, vFrom.y ), ImVec2( vFrom.x + i * fBucketWidth, vFrom.y + oSize.y ), ImColor( 0.8f, 0.8f, 0.8f, 1.f ) );
		else
			ImGui::GetWindowDrawList()->AddLine( ImVec2( vFrom.x + i * fBucketWidth, vFrom.y ), ImVec2( vFrom.x + i * fBucketWidth, vFrom.y + oSize.y ), ImColor( 0.4f, 0.4f, 0.4f, 1.f ) );
	}

	const ImVec2 vCursorPos = ImGui::GetCursorScreenPos();
	constexpr float f60LimitMs = 1000.f * ( 1.f / 60.f );
	constexpr float f30LimitMs = 2.f * f60LimitMs;
	const float fHorizontal60Limit = vCursorPos.x + f60LimitMs * fBucketWidth;
	const float fHorizontal30Limit = vCursorPos.x + f30LimitMs * fBucketWidth;
	ImGui::GetWindowDrawList()->AddLine( ImVec2( fHorizontal60Limit, vCursorPos.y ), ImVec2( fHorizontal60Limit, vCursorPos.y + oSize.y ), ImColor( 0.f, 0.8f, 0.f, 1.f ) );
	ImGui::GetWindowDrawList()->AddLine( ImVec2( fHorizontal30Limit, vCursorPos.y ), ImVec2( fHorizontal30Limit, vCursorPos.y + oSize.y ), ImColor( 0.8f, 0.f, 0.f, 1.f ) );
}

ImVec2 Profiler::DrawBlock( const char* sName, const char* sTooltip, const float fStart, const float fEnd, const int iDepth )
{
	const float fWidth = std::floorf( fEnd ) - std::floorf( fStart );
	const float fHeight = 20.f;

	const ImVec2 vCursorPos = ImGui::GetCursorScreenPos();
	const ImVec2 vFrom( std::floorf( fStart ) + vCursorPos.x, iDepth * fHeight + vCursorPos.y );
	const ImVec2 vTo( fWidth + vFrom.x, fHeight + vFrom.y );
	const ImVec2 vTextSize = ImGui::CalcTextSize( sName );
	const ImVec2 vTextPos( vFrom.x + ( fWidth - vTextSize.x ) / 2.f, vFrom.y + ( fHeight - vTextSize.y ) / 2.f );

	const uint uColorIndex = GenerateColorIndex( sName );

	ImGui::PushClipRect( vFrom, vTo, true );
	ImGui::GetWindowDrawList()->AddRectFilled( vFrom, vTo, BACKGROUND_COLORS[ uColorIndex ] );
	ImGui::GetWindowDrawList()->AddText( vTextPos, TEXT_COLORS[ uColorIndex ], sName );
	ImGui::GetWindowDrawList()->AddRect( vFrom, vTo, BORDER_COLORS[ uColorIndex ] );
	ImGui::PopClipRect();

	if( ImGui::IsMouseHoveringRect( vFrom, vTo ) )
		ImGui::SetTooltip( sTooltip );

	return vTo;
}