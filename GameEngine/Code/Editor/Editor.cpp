#include "Editor.h"

#include "Game/GameEngine.h"
#include "Core/Profiler.h"
#include "Game/InputHandler.h"
#include "Game/Scene.h"

Editor::Editor()
	: m_bDisplayEditor( false )
{
}

void Editor::Display()
{
	ProfilerBlock oBlock( "Profiler" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_EDITOR ) )
		m_bDisplayEditor = !m_bDisplayEditor;

	if( m_bDisplayEditor )
	{
		ImGui::Begin( "Editor" );

		for( const auto it : g_pGameEngine->GetScene().m_mEntities )
			ImGui::BulletText( it.second.GetName() );

		ImGui::End();
	}
}
