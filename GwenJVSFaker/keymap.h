#ifndef GWENJVS_KEYMAP_H
#define GWENJVS_KEYMAP_H

#include "config.h"

namespace GwenJVS {

// KB scancodes
const WORD kKbNode0Player0[kNumButtons] = {
	0x04,	// test		3
	0x0D,	// start	=
	0x02,	// service	1
	0xC8,	// up		up
	0xD0,	// down		down
	0xCB,	// left		left
	0xCD,	// right	right
	0x06,	// btn1		5
	0x07,	// btn2		6
	0x08,	// btn3		7
	0x09,	// btn4		8
	0x0A,	// btn5		9
	0x0B	// btn6		0
};

const WORD kKbNode0Player1[kNumButtons] = {
	0x04,	// test		3
	0x2B,	// start	backslash
	0x02,	// service	1
	0x1E,	// up		a
	0x1F,	// down		s
	0x20,	// left		d
	0x21,	// right	f
	0x22,	// btn1		g
	0x23,	// btn2		h
	0x24,	// btn3		j
	0x25,	// btn4		k
	0x26,	// btn5		l
	0x27	// btn6		;
};

const WORD kKbNode1Player0[kNumButtons] = {
	0x04,	// test		3
	0x0C,	// start	-
	0x03,	// service	2
	0x10,	// up		q
	0x11,	// down		w
	0x12,	// left		e
	0x13,	// right	r
	0x14,	// btn1		t
	0x15,	// btn2		y
	0x16,	// btn3		u
	0x17,	// btn4		i
	0x18,	// btn5		o
	0x19	// btn6		p
};

const WORD kKbNode1Player1[kNumButtons] = {
	0x04,	// test		3
	0x1A,	// start	[
	0x03,	// service	2
	0x2C,	// up		z
	0x2D,	// down		x
	0x2E,	// left		c
	0x2F,	// right	v
	0x30,	// btn1		b
	0x31,	// btn2		n
	0x32,	// btn3		m
	0x33,	// btn4		,
	0x34,	// btn5		.
	0x35	// btn6		/
};

const static WORD *kKbMapping[kNumNodes][kNumPlayersPerNode] = {
	{ kKbNode0Player0, kKbNode0Player1 },
	{ kKbNode1Player0, kKbNode1Player1 }
};

// XInput mapping
const USHORT kXInputMapping[kNumButtons] = {
	XUSB_GAMEPAD_BACK,				// test
	XUSB_GAMEPAD_START,				// start
	XUSB_GAMEPAD_RIGHT_SHOULDER,	// service
	XUSB_GAMEPAD_DPAD_UP,			// up
	XUSB_GAMEPAD_DPAD_DOWN,			// down
	XUSB_GAMEPAD_DPAD_LEFT,			// left
	XUSB_GAMEPAD_DPAD_RIGHT,		// right
	XUSB_GAMEPAD_X,					// btn1
	XUSB_GAMEPAD_Y,					// btn2
	XUSB_GAMEPAD_LEFT_SHOULDER,		// btn3
	XUSB_GAMEPAD_A,					// btn4
	XUSB_GAMEPAD_B,					// btn5
	XUSB_GAMEPAD_GUIDE				// btn6 - ignored, will be right trigger instead (guide isn't a usable button)
};

// to use LT/RT, set the mapping in xinput_mapping to XUSB_GAMEPAD_GUIDE (since it can't be used as a normal button)
// then define XINPUT_REMAP_TO_LT/RT and give the BUTTONS_ enum for that button

#undef XINPUT_REMAP_TO_RT
#define XINPUT_REMAP_TO_LT	

#ifdef XINPUT_REMAP_TO_RT
#define XINPUT_RT_BTN				kButton6
#endif

#ifdef XINPUT_REMAP_TO_LT
#define XINPUT_LT_BTN				kButton6
#endif

// DInput mapping
const USHORT kDInputMapping[kNumButtons] = {
	DS4_BUTTON_SHARE,				// test
	DS4_BUTTON_OPTIONS,				// start
	DS4_BUTTON_SHOULDER_RIGHT,		// service
	0,								// up - dpad directions are ignored as ds4 does it differently 
	0,								// down
	0,								// left
	0,								// right
	DS4_BUTTON_SQUARE,				// btn1
	DS4_BUTTON_TRIANGLE,			// btn2
	DS4_BUTTON_SHOULDER_LEFT,		// btn3
	DS4_BUTTON_CROSS,				// btn4
	DS4_BUTTON_CIRCLE,				// btn5
	DS4_BUTTON_TRIGGER_LEFT,		// btn6
};

// to use LT/RT as analogs as well as digitals
// then define DINPUT_ANALOG_LT/RT and give the BUTTONS_ enum for that button

#undef DINPUT_ANALOG_RT
#define DINPUT_ANALOG_LT	

#ifdef DINPUT_ANALOG_RT
#define DINPUT_RT_BTN				kButton6
#endif

#ifdef DINPUT_ANALOG_LT
#define DINPUT_LT_BTN				kButton6
#endif

} // namespace GwenJVS

#endif