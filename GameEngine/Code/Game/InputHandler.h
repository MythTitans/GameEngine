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

	void OnKeyEvent( const int iKey, const int iScancode, const int iAction, const int iMods );
	void OnCursorMoveEvent( const float fCursorX, const float fCursorY );

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

	KeyStatus	m_aKeyboard[ GLFW_KEY_LAST ];

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
	ACTION_TOGGLE_PROFILER
};

struct InputAction
{
	InputAction( const InputActionID uID, const uint16 uKey, const ActionType eActionType );

	InputActionID	m_uID;
	uint16			m_uKey;
	ActionType		m_eActionType;
};

struct InputActionResult
{
	InputActionResult( const InputActionID uID );

	InputActionID	m_uID;
	bool			m_bTriggered;
	bool			m_bWasTriggered;
};

class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void UpdateInputs( const InputContext& oInputContext );

	bool IsInputActionTriggered( const InputActionID uInputActionID );

private:
	Array< InputAction >		m_aInputActions;
	Array< InputActionResult >	m_aInputActionResults;
};

extern InputHandler* g_pInputHandler;