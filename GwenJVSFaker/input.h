#ifndef GWENJVS_INPUT_H
#define GWENJVS_INPUT_H

#include <string>
#include <vector>
#include <memory>

#include <cstdio>
#include <cstdint>

#include <windows.h>
#include <SetupAPI.h>
#include <io.h>

#include "config.h"
#include "helpers.h"

#define JVS_OPCODES_OK			0x01
#define JVS_OPCODES_COIN_OK		0x02
#define JVS_OPCODES_RESET		0xF0
#define JVS_OPCODES_ADDRESS		0xF1
#define JVS_OPCODES_BROADCAST	0xFF
#define JVS_OPCODES_SYNC		0xE0
#define JVS_OPCODES_ESCAPE		0xD0

namespace GwenJVS {

// bitmask for each button
const uint16_t kJvsButtons[kNumButtons]{
	0x80,
	0x80,
	0x40,
	0x20,
	0x10,
	0x08,
	0x04,
	0x02,
	0x01,
	0x8000,
	0x4000,
	0x2000,
	0x1000
};

// request blocks
const uint8_t kInputRequest[kNumNodes][3] = {
	{ 0x20, 0x02, 0x02 },
	{ 0x20, 0x02, 0x02 }
};

const uint8_t kCoinRequest[kNumNodes][3] = {
	{ 0x21, 0x02, 0x01 },
	{ 0x21, 0x02, 0x01 }
};

// structs for input state
#pragma pack(push, 1)
typedef union {
	struct {
		bool test;
		bool start;
		bool service;
		bool up;
		bool down;
		bool left;
		bool right;
		bool btn1;
		bool btn2;
		bool btn3;
		bool btn4;
		bool btn5;
		bool btn6;
		bool unused_1;
		bool unused_2;
		bool unused_3;
	} buttons;
	uint16_t bits;
} Player, *pPlayer;
#pragma pack(pop)

typedef struct {
	Player players[kNumPlayersPerNode];
	Player players_previous[kNumPlayersPerNode];
	uint8_t coin;

	LARGE_INTEGER poll_previous;
	LARGE_INTEGER poll_now;
	LARGE_INTEGER idle_start;
	LARGE_INTEGER idle_now;
	LARGE_INTEGER frequency;
} JvsNode, *pJvsNode;

class Input {
private:
	HANDLE com_port_handle_= nullptr;
	HANDLE log_file_handle_ = nullptr;
	bool log_error_ = false;
	std::unique_ptr<JvsNode[]> jkey_ = nullptr;

public:
	// internal setup/helper functions
	void SetupJVS();
	bool ReadJVS(uint8_t* buffer, int size);
	bool WriteJVS(uint8_t dest, const uint8_t* data, int size);

	bool CheckTest(uint16_t& player_bits, uint8_t& input);
	bool CheckButtons(uint16_t& player_bits, uint16_t& input);

	Input(HANDLE com_port, HANDLE log_file, bool log_error) : com_port_handle_(com_port), log_file_handle_(log_file), log_error_(log_error) { SetupJVS(); }
	bool Initialize();
	bool IdleTimeout();
	bool ReadButtons();
	std::vector<std::vector<uint16_t>> get_players();
	std::vector<std::vector<uint16_t>> get_players_previous();
};

} // namespace GwenJVS

#endif