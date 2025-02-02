#include "MemoryTracker.h"

#include "Game/InputHandler.h"
#include "Profiler.h"

MemoryTracker* g_pMemoryTracker = nullptr;

MemoryTracker::MemoryTracker()
	: m_bDisplayMemoryTracker( false )
{
}

void MemoryTracker::Display()
{
	ProfilerBlock oBlock( "Memory tracker" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_MEMORY_TRACKER ) )
		m_bDisplayMemoryTracker = !m_bDisplayMemoryTracker;

	if( m_bDisplayMemoryTracker )
	{
		ImGui::Begin( "Memory tracker" );
		ImGui::End();
	}
}
