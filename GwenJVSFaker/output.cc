#include "output.h"

namespace GwenJVS {

bool Output::PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous) {
	for (auto node = 0; node < kNumNodes; ++node) {
		for (auto player = 0; player < kNumPlayersPerNode; ++player)
			Send(node, player, players[node][player], players_previous[node][player]);
	}
	return true;
}

/* 
void KeyboardOutput::Init() {
	for (auto i = 0; i < kNumButtons; ++i) {
		ip_[i].type = INPUT_KEYBOARD;
		ip_[i].ki.dwFlags = KEYEVENTF_SCANCODE;
		ip_[i].ki.wScan = 0;
		ip_[i].ki.time = 0;
		ip_[i].ki.dwExtraInfo = 0;
	}
}
*/

bool KeyboardOutput::Send(int node, int player, uint16_t output, uint16_t previous_output) {
	std::vector<INPUT> keys;

	for (auto i = 0; i < kNumButtons; ++i) {
		INPUT ip = { 0 };
		ip.type = INPUT_KEYBOARD;
		ip.ki.dwFlags = KEYEVENTF_SCANCODE;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		if (output & static_cast<uint16_t>(1 << i)) {
			// check if we're using an extended key
			ip.ki.dwFlags |= (kKbMapping[node][player][i] > 0x58 ? KEYEVENTF_EXTENDEDKEY : 0);
			ip.ki.wScan = kKbMapping[node][player][i];
			keys.emplace_back(ip);
		}
		else if ((previous_output & ~output) & static_cast<uint16_t>(1 << i)) { // mask off previous input with current to know what was released
			// check if we're using an extended key
			ip.ki.dwFlags |= KEYEVENTF_KEYUP | (kKbMapping[node][player][i] > 0x58 ? KEYEVENTF_EXTENDEDKEY : 0);
			ip.ki.wScan = kKbMapping[node][player][i];
			keys.emplace_back(ip);
		}
	}

	SendInput(keys.size(), keys.data(), sizeof(INPUT));
	return true;
}

void XOutput::Init() {
	xoutput_[0][0].client = vigem_alloc();
	vigem_connect(xoutput_[0][0].client);
	for (auto i = (kNumNodes - 1); i >= 0; --i) {
		for (auto j = 0; j < kNumPlayersPerNode; j++) {
			xoutput_[i][j].client = xoutput_[0][0].client;
			xoutput_[i][j].target = vigem_target_x360_alloc(); // one for each node
			vigem_target_add(xoutput_[i][j].client, xoutput_[i][j].target);
		}
	}
}

bool XOutput::Send(int node, int player, uint16_t output, uint16_t previous_output) {
	USHORT buttons = 0;

	for (auto i = 0; i < kNumButtons; ++i) {
		if (output & static_cast<uint16_t>(1 << i))
			buttons |= kXInputMapping[i];
	}

	// ignore the xinput_mapping return for the button specified for XINPUT_REMAP_TO_RT/LT since it's actually going to the trigger
#ifdef XINPUT_REMAP_TO_RT
	bool rt_trigger = ((buttons & kXInputMapping[XINPUT_RT_BTN]) > 0);
	buttons &= ~(kXInputMapping[XINPUT_RT_BTN]);
#endif
#ifdef XINPUT_REMAP_TO_LT
	bool lt_trigger = ((buttons & kXInputMapping[XINPUT_LT_BTN]) > 0);
	buttons &= ~(kXInputMapping[XINPUT_LT_BTN]);
#endif

	// build report and send it
	XUSB_REPORT report = {
		buttons,
#ifdef XINPUT_REMAP_TO_LT
		(lt_trigger ? (BYTE)0xFF : (BYTE)0),
#else
		static_cast<BYTE>(0),
#endif
#ifdef XINPUT_REMAP_TO_RT
		(rt_trigger ? static_cast<BYTE>(0xFF) : static_cast<BYTE>(0)),
#else
		(BYTE)0,
#endif
		static_cast<SHORT>(0),
		static_cast<SHORT>(0),
		static_cast<SHORT>(0),
		static_cast<SHORT>(0)
	};

	vigem_target_x360_update(xoutput_[node][player].client, xoutput_[node][player].target, report);
	return true;
}

/*
bool XOutput::PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous) {
	for (auto node = 0; node < kNumNodes; ++node) {
		// XOutput doesn't support multiple players per node atm, so just mash them all together into one report
		uint16_t output = 0;
		for (auto player = 0; player < kNumPlayersPerNode; ++player) {
			output |= players[node][player];
		}
		Send(node, 0, output);
	}
	return true;
}
*/

void DOutput::Init() {
	doutput_[0][0].client = vigem_alloc();
	vigem_connect(doutput_[0][0].client);
	for (auto i = (kNumNodes - 1); i >= 0; --i) {
		for (auto j = 0; j < kNumPlayersPerNode; ++j) {
			doutput_[i][j].client = doutput_[0][0].client;
			doutput_[i][j].target = vigem_target_ds4_alloc(); // one for each node
			vigem_target_add(doutput_[i][j].client, doutput_[i][j].target);
		}
	}
}

bool DOutput::Send(int node, int player, uint16_t output, uint16_t previous_output) {
	USHORT buttons = 0;

	// skip arrows
	for (uint16_t i = kTestButton; i < kUpStick; ++i) {
		if (output & static_cast<uint16_t>(1 << i))
			buttons |= kDInputMapping[i];
	}

	for (uint16_t i = kButton1; i < kNumButtons; ++i) {
		if (output & static_cast<uint16_t>(1 << i))
			buttons |= kDInputMapping[i];
	}

	// time to arrow
	buttons &= static_cast<USHORT>(~0xF);
	int i = 0;
	for (i = 0; i < (sizeof(kDs4Directions) / sizeof(USHORT)); ++i) {
		if (output & kDs4DpadBitmask[i]) {
			buttons |= kDs4Directions[i];
			break;
		}
	}
	buttons = (buttons & static_cast<USHORT>(~DS4_BUTTON_DPAD_NONE)) | (-(static_cast<USHORT>(i == 8)) & static_cast<USHORT>(DS4_BUTTON_DPAD_NONE));

	// if we want analog reports too
#ifdef DINPUT_ANALOG_RT
	bool rt_trigger = ((buttons & kDInputMapping[DINPUT_RT_BTN]) > 0);
#endif
#ifdef DINPUT_ANALOG_LT
	bool lt_trigger = ((buttons & kDInputMapping[DINPUT_LT_BTN]) > 0);
#endif

	// build report and send it
	DS4_REPORT report = {
		static_cast<BYTE>(0),
		static_cast<BYTE>(0),
		static_cast<BYTE>(0),
		static_cast<BYTE>(0),
		buttons,
		static_cast<BYTE>(0),
#ifdef DINPUT_ANALOG_LT
		(lt_trigger ? (BYTE)0xFF : (BYTE)0),
#else
		static_cast<BYTE>(0),
#endif
#ifdef DINPUT_ANALOG_RT
		(rt_trigger ? static_cast<BYTE>(0xFF) : static_cast<BYTE>(0)),
#else
		(BYTE)0,
#endif
	};

	vigem_target_ds4_update(doutput_[node][player].client, doutput_[node][player].target, report);
	return true;
}

/* 
bool DOutput::PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous) {
	for (auto node = 0; node < kNumNodes; ++node) {
		// DOutput doesn't support multiple players per node atm, so just mash them all together into one report
		uint16_t output = 0;
		for (auto player = 0; player < kNumPlayersPerNode; ++player) {
			output |= players[node][player];
		}
		Send(node, 0, output);
	}
	return true;
}
*/

} // namespace GwenJVS