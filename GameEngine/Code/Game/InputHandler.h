#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Array.h"
#include "Core/Types.h"

class InputContext
{
public:
	friend class InputHandler;

	void					Refresh();

	void					OnKeyEvent( const int iKey, const int iScancode, const int iAction, const int iMods );
	void					OnCursorMoveEvent( const float fCursorX, const float fCursorY );

private:
	struct KeyStatus
	{
		KeyStatus();

		bool m_bPressed : 1;
		bool m_bRepeat : 1;
		bool m_bShift : 1;
		bool m_bControl : 1;
		bool m_bAlt : 1;
		bool m_bSuper : 1;
		bool m_bCapsLock : 1;
		bool m_bNumLock : 1;
	};

	KeyStatus			m_aKeyboard[ GLFW_KEY_LAST + 1 ];
	GLFWgamepadstate	m_oGamepad;

	float		m_fCursorX;
	float		m_fCursorY;
};

enum class ActionType : uint8
{
	PRESSED,
	PRESSING,
	RELEASED,
	REPEATED
};

enum class InputActionID : uint16
{
	ACTION_TOGGLE_EDITOR,
	ACTION_TOGGLE_PROFILER
};

struct InputAction
{
	InputAction( const InputActionID uID, const uint16 uKey, const uint8 uButton, const ActionType eActionType );

	static InputAction KeyboardAction( const InputActionID uID, const uint16 uKey, const ActionType eActionType );
	static InputAction ButtonAction( const InputActionID uID, const uint8 uButton, const ActionType eActionType );

	InputActionID	m_uID;
	uint16			m_uKey;
	uint8			m_uButton;
	ActionType		m_eActionType;
};

struct InputActionResult
{
	explicit InputActionResult( const InputActionID uID );

	InputActionID	m_uID;
	bool			m_bTriggered;
	bool			m_bWasTriggered;
};

enum class AxisActionID : uint16
{
	ACTION_MOVE_LEFT,
	ACTION_MOVE_RIGHT,
	ACTION_MOVE_FORWARD,
	ACTION_MOVE_BACKWARD,
	ACTION_LOOK_LEFT,
	ACTION_LOOK_RIGHT,
	ACTION_LOOK_UP,
	ACTION_LOOK_DOWN
};

struct AxisAction
{
	AxisAction( const AxisActionID uID, const uint8 uAxis, const bool bNegative = false, const float fDeadThreshold = 0.2f, const float fTriggerTreshold = 0.8f );

	AxisActionID	m_uID;
	uint8			m_uAxis;
	bool			m_bNegative;
	float			m_fDeadThreshold;
	float			m_fTriggerTreshold;
};

struct AxisActionResult
{
	explicit AxisActionResult( const AxisActionID uID );

	AxisActionID	m_uID;
	bool			m_bTriggered;
	bool			m_bWasTriggered;
	float			m_fValue;
};

class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void	UpdateInputs( const InputContext& oInputContext );

	bool	IsInputActionTriggered( const InputActionID uInputActionID );
	bool	IsAxisActionTriggered( const AxisActionID uAxisActionID );
	float	GetAxisActionValue( const AxisActionID uAxisActionID );

private:
	Array< InputAction >		m_aInputActions;
	Array< InputActionResult >	m_aInputActionResults;

	Array< AxisAction >			m_aAxisActions;
	Array< AxisActionResult >	m_aAxisActionResults;
};

extern InputHandler* g_pInputHandler;