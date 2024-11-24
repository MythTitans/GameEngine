#include "Profiler.h"

#include <format>

ProfilerBlock::ProfilerBlock( const char* sName )
	: m_uBlockID( g_pProfiler->StartBlock( sName ) )
{
}

ProfilerBlock::~ProfilerBlock()
{
	g_pProfiler->EndBlock( m_uBlockID );
}

Profiler* g_pProfiler = nullptr;

Profiler::Profiler()
	: m_uBlocksDepth( 0 )
	, m_bDisplayProfiler( false )
{
	g_pProfiler = this;
}

Profiler::~Profiler()
{
	g_pProfiler = nullptr;
}

void Profiler::NewFrame()
{
	m_oFrameStart = std::chrono::high_resolution_clock::now();

	m_aBlocks.Clear();
	m_uBlocksDepth = 0;
}

void Profiler::Display()
{
	m_bDisplayProfiler = true; // TODO #eric
	if( m_bDisplayProfiler )
	{
		ImGui::Begin( "Profiler" );

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

		ImGui::Text( "\n Main thread" );

		float fMaxX = 0.f;
		for( const Block& oBlock : m_aBlocks )
		{
			const uint64 uStartMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oStart - m_oFrameStart ).count();
			const float fStartMilliSeconds = uStartMicroSeconds / 1000.f;

			const uint64 uEndMicroSeconds = std::chrono::duration_cast< std::chrono::microseconds >( oBlock.m_oEnd - m_oFrameStart ).count();
			const float fEndMilliSeconds = uEndMicroSeconds / 1000.f;

			const float fStart = fStartMilliSeconds * fReferenceWidth;
			const float fEnd = fEndMilliSeconds * fReferenceWidth;

			if( fMaxX < fEnd )
				fMaxX = fEnd;

			DrawBlock( std::format( "{} ({:.3f} ms)", oBlock.m_sName, fEndMilliSeconds - fStartMilliSeconds ).c_str(), ImColor( 0.5f, 0.5f, 0.f, 1.f ), fStart, fEnd, oBlock.m_uDepth );
		}

		ImGui::SetCursorScreenPos( ImVec2( ImGui::GetCursorScreenPos().x + fMaxX, ImGui::GetCursorScreenPos().y ) );

		ImGui::EndChild();

		ImGui::End();
	}
}

uint Profiler::StartBlock( const char* sName )
{
	const uint uID = m_aBlocks.Count();
	m_aBlocks.PushBack( Block( sName, *this ) );
	++m_uBlocksDepth;
	return uID;
}

void Profiler::EndBlock( const uint uBlockID )
{
	m_aBlocks[ uBlockID ].m_oEnd = std::chrono::high_resolution_clock::now();
	--m_uBlocksDepth;
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

void Profiler::DrawBlock( const char* sName, const ImColor& oColor, const float fStart, const float fEnd, const int iDepth )
{
	const float fWidth = std::floorf( fEnd ) - std::floorf( fStart );
	const float fHeight = 20.f;

	const ImVec2 vCursorPos = ImGui::GetCursorScreenPos();
	const ImVec2 vFrom( std::floorf( fStart ) + vCursorPos.x, iDepth * fHeight + vCursorPos.y );
	const ImVec2 vTo( fWidth + vFrom.x, fHeight + vFrom.y );
	const ImVec2 vTextSize = ImGui::CalcTextSize( sName );
	const ImVec2 vTextPos( vFrom.x + ( fWidth - vTextSize.x ) / 2.f, vFrom.y + ( fHeight - vTextSize.y ) / 2.f );

	ImGui::PushClipRect( vFrom, vTo, true );
	ImGui::GetWindowDrawList()->AddRectFilled( vFrom, vTo, oColor );
	ImGui::GetWindowDrawList()->AddText( vTextPos, IM_COL32_WHITE, sName );
	ImGui::GetWindowDrawList()->AddRect( vFrom, vTo, IM_COL32_WHITE );
	ImGui::PopClipRect();

	if( ImGui::IsMouseHoveringRect( vFrom, vTo ) )
		ImGui::SetTooltip( sName );
}
