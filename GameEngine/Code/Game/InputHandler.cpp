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

void InputContext::Refresh()
{
	ProfilerBlock oBlock( "InputContext" );

	glfwPollEvents();
	glfwGetGamepadState( GLFW_JOYSTICK_1, &m_oGamepad );
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

InputAction::InputAction( const InputActionID uID, const uint16 uKey, const uint8 uButton, const ActionType eActionType )
	: m_uID( uID )
	, m_uKey( uKey )
	, m_uButton( uButton )
	, m_eActionType( eActionType )
{
}

InputAction InputAction::KeyboardAction( const InputActionID uID, const uint16 uKey, const ActionType eActionType )
{
	return InputAction( uID, uKey, 0xFF, eActionType );
}

InputAction InputAction::ButtonAction( const InputActionID uID, const uint8 uButton, const ActionType eActionType )
{
	return InputAction( uID, 0xFFFF, uButton, eActionType );
}

InputActionResult::InputActionResult( const InputActionID uID )
	: m_uID( uID )
	, m_bTriggered( false )
	, m_bWasTriggered( false )
{
}

AxisAction::AxisAction( const AxisActionID uID, const uint8 uAxis, const bool bNegative /*= false*/, const float fDeadThreshold /*= 0.1f*/, const float fTriggerTreshold /*= 0.9f */ )
	: m_uID( uID )
	, m_uAxis( uAxis )
	, m_bNegative( bNegative )
	, m_fDeadThreshold( fDeadThreshold )
	, m_fTriggerTreshold( fTriggerTreshold )
{
}

AxisActionResult::AxisActionResult( const AxisActionID uID )
	: m_uID( uID )
	, m_bTriggered( false )
	, m_bWasTriggered( false )
	, m_fValue( 0.f )
{
}

InputHandler* g_pInputHandler = nullptr;

InputHandler::InputHandler()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_PROFILER, GLFW_KEY_F4, ActionType::PRESSED ) );

	m_aInputActionResults.Reserve( m_aInputActions.Count() );
	for( const InputAction& oInputAction : m_aInputActions )
		m_aInputActionResults.PushBack( InputActionResult( oInputAction.m_uID ) );

	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_MOVE_LEFT, GLFW_GAMEPAD_AXIS_LEFT_X, true ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_MOVE_RIGHT, GLFW_GAMEPAD_AXIS_LEFT_X ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_MOVE_FORWARD, GLFW_GAMEPAD_AXIS_LEFT_Y, true ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_MOVE_BACKWARD, GLFW_GAMEPAD_AXIS_LEFT_Y ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_LOOK_LEFT, GLFW_GAMEPAD_AXIS_RIGHT_X, true ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_LOOK_RIGHT, GLFW_GAMEPAD_AXIS_RIGHT_X ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_LOOK_UP, GLFW_GAMEPAD_AXIS_RIGHT_Y, true ) );
	m_aAxisActions.PushBack( AxisAction( AxisActionID::ACTION_LOOK_DOWN, GLFW_GAMEPAD_AXIS_RIGHT_Y ) );

	m_aAxisActionResults.Reserve( m_aAxisActions.Count() );
	for( const AxisAction& oAxisAction : m_aAxisActions )
		m_aAxisActionResults.PushBack( AxisActionResult( oAxisAction.m_uID ) );

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

		bool bPressed = false;
		if( oInputAction.m_uKey != 0xFFFF )
			bPressed |= oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed;
		if( oInputAction.m_uButton != 0xFF )
			bPressed |= oInputContext.m_oGamepad.buttons[ oInputAction.m_uButton ] == GLFW_PRESS;

		switch( oInputAction.m_eActionType )
		{
		case ActionType::PRESSED:
			oInputActionResult.m_bTriggered = bPressed && oInputActionResult.m_bWasTriggered == false;
			oInputActionResult.m_bWasTriggered = bPressed;
			break;
		case ActionType::PRESSING:
			oInputActionResult.m_bTriggered = bPressed;
			break;
		case ActionType::RELEASED:
			oInputActionResult.m_bTriggered = bPressed == false && oInputActionResult.m_bWasTriggered == false;
			oInputActionResult.m_bWasTriggered = bPressed == false;
			break;
		case ActionType::REPEATED:
			if( oInputAction.m_uKey != 0xFFFF )
				oInputActionResult.m_bTriggered = oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bRepeat;
			break;
		}
	}

	for( uint u = 0; u < m_aAxisActions.Count(); ++u )
	{
		const AxisAction& oAxisAction = m_aAxisActions[ u ];
		AxisActionResult& oAxisActionResult = m_aAxisActionResults[ u ];

		const float fValue = oInputContext.m_oGamepad.axes[ oAxisAction.m_uAxis ];
		if( ( oAxisAction.m_bNegative == false && fValue > oAxisAction.m_fDeadThreshold ) || ( oAxisAction.m_bNegative && fValue < -oAxisAction.m_fDeadThreshold ) )
			oAxisActionResult.m_fValue = oAxisAction.m_bNegative ? -fValue : fValue;
		else
			oAxisActionResult.m_fValue = 0.f;

		const bool bTriggered = ( oAxisAction.m_bNegative == false && fValue > oAxisAction.m_fTriggerTreshold ) || ( oAxisAction.m_bNegative && fValue < -oAxisAction.m_fTriggerTreshold );

		oAxisActionResult.m_bTriggered = bTriggered && oAxisActionResult.m_bWasTriggered == false;
		oAxisActionResult.m_bWasTriggered = bTriggered;
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

bool InputHandler::IsAxisActionTriggered( const AxisActionID uAxisActionID )
{
	for( const AxisActionResult& oAxisActionResult : m_aAxisActionResults )
	{
		if( oAxisActionResult.m_uID == uAxisActionID )
			return oAxisActionResult.m_bTriggered;
	}

	return false;
}

float InputHandler::GetAxisActionValue( const AxisActionID uAxisActionID )
{
	for( const AxisActionResult& oAxisActionResult : m_aAxisActionResults )
	{
		if( oAxisActionResult.m_uID == uAxisActionID )
			return oAxisActionResult.m_fValue;
	}

	return 0.f;
}