#pragma once

#include "common.h"

struct CControllerState
{
	short LEFTSTICKX;
	short LEFTSTICKY;
	short RIGHTSTICKX;
	short RIGHTSTICKY;

	short LEFTSHOULDER1;
	short LEFTSHOULDER2;
	short RIGHTSHOULDER1;
	short RIGHTSHOULDER2;

	short DPADUP;
	short DPADDOWN;
	short DPADLEFT;
	short DPADRIGHT;

	short unk1;
	short unk2;
	short unk3;
	short unk4;

	short START;
	short SELECT;

	short SQUARE;
	short TRIANGLE;
	short CROSS;
	short CIRCLE;

	short LEFTSHOCK;
	short RIGHTSHOCK;
};

struct CPad
{
	char unk[2];

	struct CControllerState NewState;
	struct CControllerState OldState;

	char pad[48];

	short               Mode; //146
	short               ShakeDur;
	short               DisablePlayerControls;
	//...
};

enum ePspControl
{
	CTRL_SELECT,
	CTRL_UP,
	CTRL_RIGHT,
	CTRL_DOWN,
	CTRL_LEFT,
	CTRL_LTRIGGER,
	CTRL_RTRIGGER,
	CTRL_TRIANGLE,
	CTRL_CIRCLE,  
	CTRL_CROSS,   
	CTRL_SQUARE,
	CTRL_HOLD,

	CTRL_STICK_UP,
	CTRL_STICK_DOWN,
	CTRL_STICK_RIGHT,
	CTRL_STICK_LEFT,

	CTRL_SIZE
};

namespace touch
{
	typedef CPad* (*fn_CPad__GetPad)(int a1);
	extern fn_CPad__GetPad CPad__GetPad;

	void psp_input_event();
	bool psp_control_pressed(ePspControl control);
	bool psp_control_pressed_timed(ePspControl control, uint32_t mintime);
	void psp_control_disable(ePspControl control, uint32_t mintime);

	void menu_button_event(bool state);
	bool menu_button_pressed();
	bool menu_button_pressed_timed(uint32_t mintime);
}
