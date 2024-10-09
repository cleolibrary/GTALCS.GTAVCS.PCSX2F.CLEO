#include "touch.h"
#include "utils.h"
#include "ui.h"

namespace touch
{
	fn_CPad__GetPad CPad__GetPad;

	bool menu_button_state = false;
	uint32_t menu_button_event_time = 0;

	void menu_button_event(bool state)
	{		
		if (menu_button_state != state)
			menu_button_event_time = utils::get_tick_count();
		menu_button_state = state;
	}

	bool menu_button_pressed()
	{
		return menu_button_state;
	}

	bool menu_button_pressed_timed(uint32_t mintime)
	{
		return menu_button_state && menu_button_event_time + mintime <= utils::get_tick_count();
	}

	bool ctrl_state[CTRL_SIZE];
	uint32_t ctrl_disabled[CTRL_SIZE];
	uint32_t ctrl_time[CTRL_SIZE];

	void control_event(ePspControl control, bool state)
	{
		if (ctrl_state[control] != state)
			ctrl_time[control] = utils::get_tick_count();
		if (!state)
			ctrl_disabled[control] = 0;
		ctrl_state[control] = state;
	}

	void psp_input_event()
	{
		CPad* Pad = touch::CPad__GetPad(0);

		menu_button_event((Pad->NewState.START) != 0);
		////if (menu_button_state && !menu_button_pressed_timed(2000))
		////	pad_data->Buttons &= ~(uint32_t)PSP_CTRL_START;
		//

		control_event(CTRL_SELECT, (Pad->NewState.SELECT) != 0);
		control_event(CTRL_UP, (Pad->NewState.DPADUP) != 0);
		control_event(CTRL_RIGHT, (Pad->NewState.DPADRIGHT) != 0);
		control_event(CTRL_DOWN, (Pad->NewState.DPADDOWN) != 0);
		control_event(CTRL_LEFT, (Pad->NewState.DPADLEFT) != 0);
		control_event(CTRL_LTRIGGER, (Pad->NewState.LEFTSHOULDER1) != 0);
		control_event(CTRL_RTRIGGER, (Pad->NewState.RIGHTSHOULDER2) != 0);
		control_event(CTRL_TRIANGLE, (Pad->NewState.TRIANGLE) != 0);
		control_event(CTRL_CIRCLE, (Pad->NewState.CIRCLE) != 0);
		control_event(CTRL_CROSS, (Pad->NewState.CROSS) != 0);
		control_event(CTRL_SQUARE, (Pad->NewState.SQUARE) != 0);
		//control_event(CTRL_HOLD, (Pad->NewState.HOLD) != 0);

		control_event(CTRL_STICK_UP, Pad->NewState.LEFTSTICKY < -10);
		control_event(CTRL_STICK_DOWN, Pad->NewState.LEFTSTICKY > 10);
		control_event(CTRL_STICK_RIGHT, Pad->NewState.LEFTSTICKX > 10);
		control_event(CTRL_STICK_LEFT, Pad->NewState.LEFTSTICKX < -10);
	}

	bool psp_control_pressed(ePspControl control)
	{
		return control < CTRL_SIZE && ctrl_state[control] && ctrl_disabled[control] <= utils::get_tick_count();
	}

	bool psp_control_pressed_timed(ePspControl control, uint32_t mintime)
	{
		return control < CTRL_SIZE && ctrl_state[control] && ctrl_disabled[control] <= utils::get_tick_count() && ctrl_time[control] + mintime <= utils::get_tick_count();
	}

	void psp_control_disable(ePspControl control, uint32_t mintime)
	{
		ctrl_disabled[control] = utils::get_tick_count() + mintime;
	}
}
