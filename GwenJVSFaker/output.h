#ifndef GWENJVS_OUTPUT_H
#define GWENJVS_OUTPUT_H

#include <string>
#include <vector>

#include <cstdio>
#include <cstdint>

#include <windows.h>
#include <SetupAPI.h>
#include <io.h>
#include <dinput.h>
#include <SDKDDKVer.h>

#include "ViGEm\km\BusShared.h"
#include "ViGEm\Common.h"
#include "ViGEm\Client.h"

// #include "D:\Projects\ViGEmBus-old\include\ViGEmBusShared.h"
// #include "D:\Projects\ViGEmBus-old\include\ViGEmCommon.h"
// #include "D:\Projects\ViGEmBus-old\include\ViGEmClient.h"

#include "keymap.h"
#include "config.h"
#include "helpers.h"

namespace GwenJVS {

// for figuring out the ds4 dpad setup
const USHORT kDs4Directions[8] = {
	DS4_BUTTON_DPAD_NORTHEAST,	// up + left
	DS4_BUTTON_DPAD_SOUTHEAST,	// down + left
	DS4_BUTTON_DPAD_NORTHWEST,	// up + right
	DS4_BUTTON_DPAD_SOUTHWEST,	// down + right
	DS4_BUTTON_DPAD_NORTH,		// up
	DS4_BUTTON_DPAD_SOUTH,		// down
	DS4_BUTTON_DPAD_EAST,		// left
	DS4_BUTTON_DPAD_WEST		// right
};

const uint16_t kDs4DpadBitmask[8] = {
	(1 << 3) + (1 << 5),		// up + left
	(1 << 4) + (1 << 5),		// down + left
	(1 << 3) + (1 << 6),		// up + right
	(1 << 4) + (1 << 6),		// down + right
	(1 << 3),					// up
	(1 << 4),					// down
	(1 << 5),					// left
	(1 << 6)					// right
};

// structs for output
typedef struct {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
} XInput, *pXInput;

typedef struct {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
} DInput, *pDInput;

class Output {
private:
	virtual bool Send(int node, int player, uint16_t output, uint16_t previous_output = 0) = 0;
public:
	virtual	void Init() = 0;
	virtual bool PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous);
};

class KeyboardOutput : public Output {
private:
	bool Send(int node, int player, uint16_t output, uint16_t previous_output = 0);

public:
	void Init() {};
};

class XOutput : public Output {
private:
	XInput xoutput_[kNumNodes][kNumPlayersPerNode] = { { 0 } };
	bool Send(int node, int player, uint16_t output, uint16_t previous_output = 0);

public:
	void Init();
	// bool PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous);
};

class DOutput : public Output {
private:
	DInput doutput_[kNumNodes][kNumPlayersPerNode] = { { 0 } };
	bool Send(int node, int player, uint16_t output, uint16_t previous_output = 0);

public:
	void Init();
	// bool PressButtons(const std::vector<std::vector<uint16_t>>& players, const std::vector<std::vector<uint16_t>>& players_previous);
};

} // namespace GwenJVS

#endif