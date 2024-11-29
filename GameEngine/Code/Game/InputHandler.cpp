#include "InputHandler.h"

#include "Core/Logger.h"
#include "GameEngine.h"

InputContext::KeyStatus::KeyStatus()
	: m_bPressed( false )
	, m_bRepeat( false )
	, m_bShift( false )
	, m_bControl( false )
	, m_bAlt( false )
	, m_bSuper( false )
	, m_bCapsLock( false )
	, m_bNumLock( false )
{
}

void InputContext::OnKeyEvent( const int iKey, const int iScancode, const int iAction, const int iMods )
{
	m_aKeyboard[ iKey ].m_bPressed = ( iAction == GLFW_PRESS || iAction == GLFW_REPEAT );
	m_aKeyboard[ iKey ].m_bRepeat = iAction == GLFW_REPEAT;
	m_aKeyboard[ iKey ].m_bShift = ( ( iMods & GLFW_MOD_SHIFT ) != 0 );
	m_aKeyboard[ iKey ].m_bControl = ( ( iMods & GLFW_MOD_CONTROL ) != 0 );
	m_aKeyboard[ iKey ].m_bAlt = ( ( iMods & GLFW_MOD_ALT ) != 0 );
	m_aKeyboard[ iKey ].m_bSuper = ( ( iMods & GLFW_MOD_SUPER ) != 0 );
	m_aKeyboard[ iKey ].m_bCapsLock = ( ( iMods & GLFW_MOD_CAPS_LOCK ) != 0 );
	m_aKeyboard[ iKey ].m_bNumLock = ( ( iMods & GLFW_MOD_NUM_LOCK ) != 0 );
}

void InputContext::OnCursorMoveEvent( const float fCursorX, const float fCursorY )
{
	m_fCursorX = fCursorX;
	m_fCursorY = fCursorY;
}

InputAction::InputAction( const InputActionID uID, const uint16 uKey, const ActionType eActionType )
	: m_uID( uID )
	, m_uKey( uKey )
	, m_eActionType( eActionType )
{
}

InputActionResult::InputActionResult( const InputActionID uID )
	: m_uID( uID )
	, m_bTriggered( false )
	, m_bWasTriggered( false )
{
}

InputHandler* g_pInputHandler = nullptr;

InputHandler::InputHandler()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	m_aInputActions.PushBack( InputAction( InputActionID::ACTION_TOGGLE_PROFILER, GLFW_KEY_F4, ActionType::PRESSED ) );

	m_aInputActionResults.Reserve( m_aInputActions.Count() );
	for( const InputAction& oInputAction : m_aInputActions )
		m_aInputActionResults.PushBack( InputActionResult( oInputAction.m_uID ) );

	g_pInputHandler = this;
}

InputHandler::~InputHandler()
{
	g_pInputHandler = nullptr;
}

void InputHandler::UpdateInputs( const InputContext& oInputContext )
{
	ProfilerBlock oBlock( "Inputs" );

	for( uint u = 0; u < m_aInputActions.Count(); ++u )
	{
		const InputAction& oInputAction = m_aInputActions[ u ];
		InputActionResult& oInputActionResult = m_aInputActionResults[ u ];

		switch( oInputAction.m_eActionType )
		{
		case ActionType::PRESSED:
			oInputActionResult.m_bTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed && oInputActionResult.m_bWasTriggered == false;
			oInputActionResult.m_bWasTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed;
			break;
		case ActionType::PRESSING:
			oInputActionResult.m_bTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed;
			break;
		case ActionType::RELEASED:
			oInputActionResult.m_bTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed == false && oInputActionResult.m_bWasTriggered == false;
			oInputActionResult.m_bWasTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed == false;
			break;
		case ActionType::REPEATED:
			oInputActionResult.m_bTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bRepeat;
			break;
		}
	}
}

bool InputHandler::IsInputActionTriggered( InputActionID uInputActionID )
{
	for( const InputActionResult& oInputActionResult : m_aInputActionResults )
	{
		if( oInputActionResult.m_uID == uInputActionID )
			return oInputActionResult.m_bTriggered;
	}

	return false;
}
