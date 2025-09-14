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

void InputContext::OnMouseEvent( const int iButton, const int iAction, const int iMods )
{
	m_aMouse[ iButton ].m_bPressed = iAction == GLFW_PRESS;
	m_aMouse[ iButton ].m_bRepeat = false;
	m_aMouse[ iButton ].m_bShift = ( ( iMods & GLFW_MOD_SHIFT ) != 0 );
	m_aMouse[ iButton ].m_bControl = ( ( iMods & GLFW_MOD_CONTROL ) != 0 );
	m_aMouse[ iButton ].m_bAlt = ( ( iMods & GLFW_MOD_ALT ) != 0 );
	m_aMouse[ iButton ].m_bSuper = ( ( iMods & GLFW_MOD_SUPER ) != 0 );
	m_aMouse[ iButton ].m_bCapsLock = ( ( iMods & GLFW_MOD_CAPS_LOCK ) != 0 );
	m_aMouse[ iButton ].m_bNumLock = ( ( iMods & GLFW_MOD_NUM_LOCK ) != 0 );
}

void InputContext::OnCursorMoveEvent( const float fCursorX, const float fCursorY )
{
	m_fCursorX = fCursorX;
	m_fCursorY = fCursorY;
}

int InputContext::GetCursorX() const
{
	return ( int )m_fCursorX;
}

int InputContext::GetCursorY() const
{
	return ( int )m_fCursorY;
}

InputAction::InputAction( const InputActionID uID, const uint16 uKey, const uint8 uButton, const ActionType eActionType, const DeviceType eDeviceType, const uint8 uKeyModifierFlags )
	: m_uID( uID )
	, m_uKey( uKey )
	, m_uButton( uButton )
	, m_eActionType( eActionType )
	, m_eDeviceType( eDeviceType )
	, m_uKeyModifierFlags( uKeyModifierFlags )
{
}

InputAction InputAction::KeyboardAction( const InputActionID uID, const uint16 uKey, const ActionType eActionType, const uint8 uKeyModifierFlags /*= KeyModifier::NONE*/ )
{
	return InputAction( uID, uKey, 0xFF, eActionType, DeviceType::KEYBOARD, uKeyModifierFlags );
}

InputAction InputAction::ButtonAction( const InputActionID uID, const uint8 uButton, const ActionType eActionType, const DeviceType eDeviceType )
{
	return InputAction( uID, 0xFFFF, uButton, eActionType, eDeviceType, KeyModifier::NONE );
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
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_EDITOR, GLFW_KEY_F1, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_RENDERER_DEBUG, GLFW_KEY_F2, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_MEMORY_TRACKER, GLFW_KEY_F3, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_PROFILER, GLFW_KEY_F4, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_TOGGLE_RESOURCES_DEBUG, GLFW_KEY_F5, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::ButtonAction( InputActionID::ACTION_MOUSE_LEFT_PRESS, GLFW_MOUSE_BUTTON_LEFT, ActionType::PRESSED, DeviceType::MOUSE ) );
	m_aInputActions.PushBack( InputAction::ButtonAction( InputActionID::ACTION_MOUSE_LEFT_PRESSING, GLFW_MOUSE_BUTTON_LEFT, ActionType::PRESSING, DeviceType::MOUSE ) );
	m_aInputActions.PushBack( InputAction::ButtonAction( InputActionID::ACTION_MOUSE_LEFT_RELEASE, GLFW_MOUSE_BUTTON_LEFT, ActionType::RELEASED, DeviceType::MOUSE ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_REDO, GLFW_KEY_Y, ActionType::PRESSED, KeyModifier::CONTROL ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_UNDO, GLFW_KEY_W, ActionType::PRESSED, KeyModifier::CONTROL ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_DELETE, GLFW_KEY_DELETE, ActionType::PRESSED ) );
	m_aInputActions.PushBack( InputAction::KeyboardAction( InputActionID::ACTION_DUPLICATE, GLFW_KEY_D, ActionType::PRESSED, KeyModifier::CONTROL ) );
	m_aInputActions.PushBack( InputAction::ButtonAction( InputActionID::ACTION_FAST_FREECAM, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, ActionType::PRESSING, DeviceType::GAMEPAD ) );

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
		{
			uint8 uModifierFlags = 0;
			uModifierFlags |= oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bControl ? KeyModifier::CONTROL : 0;
			uModifierFlags |= oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bAlt ? KeyModifier::ALT : 0;
			uModifierFlags |= oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bShift ? KeyModifier::SHIFT : 0;

			const bool bMatchModifiers = oInputAction.m_uKeyModifierFlags == uModifierFlags;
			bPressed |= oInputContext.m_aKeyboard[ oInputAction.m_uKey ].m_bPressed && bMatchModifiers;
		}

		if( oInputAction.m_uButton != 0xFF )
		{
			switch( oInputAction.m_eDeviceType )
			{
			case DeviceType::GAMEPAD:
				bPressed |= oInputContext.m_oGamepad.buttons[ oInputAction.m_uButton ] == GLFW_PRESS;
				break;
			case DeviceType::MOUSE:
				bPressed |= oInputContext.m_aMouse[ oInputAction.m_uButton ].m_bPressed;
				break;
			}
		}

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
