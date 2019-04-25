#include "input.h"

namespace GwenJVS {

void Input::SetupJVS() {
	DCB dcb = { 0 };
	COMMTIMEOUTS com_timeouts = { MAXDWORD, 0, 0, 0, 0 };
	COMSTAT com_stat = { 0 };
	DWORD errors = NULL;
	wchar_t ltext[256] = { 0 };

	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	ClearCommError(com_port_handle_, &errors, &com_stat);

	if (!GetCommState(com_port_handle_, &dcb)) {
		if (log_error_)
			LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"SetupJVS - GetCommState failed with error %d.\r\n", GetLastError()));
		throw;
	}

	SetupComm(com_port_handle_, 516, 516);

	GetCommState(com_port_handle_, &dcb);

	LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\r\n", dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits));
	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	dcb.EvtChar = '\xE0';

	SetCommState(com_port_handle_, &dcb);
	GetCommState(com_port_handle_, &dcb);
	LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\r\n", dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits));

	SetCommMask(com_port_handle_, EV_RXCHAR);
	SetCommTimeouts(com_port_handle_, &com_timeouts);
}

bool Input::Initialize() {
	uint8_t reset[] = { static_cast<uint8_t>(JVS_OPCODES_RESET), 0xD9 };
	uint8_t jdata[256] = { 0 };
	wchar_t ltext[256] = { 0 };

	// free the jkey memory if it exists
	if (jkey_ != nullptr) {
		jkey_.release();
	}

	if (!ClearCommError(com_port_handle_, NULL, 0)) {
		if (log_error_)
			LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ClearCommError()!\r\n"));
		return false;
	}

	if (!WriteJVS(static_cast<uint8_t>(JVS_OPCODES_BROADCAST), reset, 2))
		return false;

	jkey_ = std::make_unique<JvsNode[]>(kNumNodes);
	QueryPerformanceFrequency(&jkey_[0].frequency);
	QueryPerformanceCounter(&jkey_[0].idle_start);

	for (auto i = 0; i < kNumNodes; ++i) {
		jkey_[i].frequency = jkey_[0].frequency;
		jkey_[i].idle_start = jkey_[0].idle_start;
		jkey_[i].idle_now = jkey_[0].idle_start;

		uint8_t set[] = { static_cast<uint8_t>(JVS_OPCODES_ADDRESS), static_cast<uint8_t>(0x01 + i) };

		// don't want to hammer the bus
		Sleep(5);

		if (!WriteJVS(static_cast<uint8_t>(JVS_OPCODES_BROADCAST), set, 2)) {
			jkey_.release();
			return false;
		}

		Sleep(5);

		if (!ReadJVS(jdata, sizeof(jdata))) {
			jkey_.release();
			return false;
		}
	}
	return true;
}

bool Input::ReadJVS(uint8_t* buffer, int size) {
	wchar_t ltext[256] = { 0 };
	uint8_t raw_frame[256] = { 0 };

	OVERLAPPED os = { 0 };
	os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	int bytes_read = 0;

	// reading loop
	DWORD event = 0;
	if (!WaitCommEvent(com_port_handle_, &event, &os)) {
		DWORD error = GetLastError();
		if (error != ERROR_IO_PENDING) {
			if (log_error_)
				LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - WaitCommEvent() error 0x%x!\r\n", error));
			CloseHandle(os.hEvent);
			return false;
		}
		else {
			DWORD result = WaitForSingleObject(os.hEvent, kReadTimeout);
			switch (result) {
				case WAIT_TIMEOUT: {
					if (log_error_)
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - WaitCommEvent() timeout exceeded!\r\n"));
					CloseHandle(os.hEvent);
					return false;
					break;
				}
				case WAIT_OBJECT_0: {
					if (event & EV_RXCHAR) {
						DWORD read = 0;
						memset(raw_frame, 0, sizeof(raw_frame));
						do {
							ReadFile(com_port_handle_, raw_frame, sizeof(raw_frame), &read, &os);
							bytes_read += read;
						} while (read > 0);
					}
					break;
				}
			}
		}
	}

	CloseHandle(os.hEvent);
#ifdef DEBUG
	LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Raw frame:\r\n\t"));
	memset(ltext, 0, sizeof(ltext));

	for (auto i = 0; i < bytes_read; ++i) {
		wchar_t debug_buffer[16] = { 0 };
		swprintf_s(debug_buffer, sizeof(debug_buffer), L"[%02x]", raw_frame[i]);
		wcscat_s(ltext, sizeof(ltext), debug_buffer);
	}
	wcscat_s(ltext, sizeof(ltext), L"\r\n");
	LogEvent(log_file_handle_, ltext, wcslen(ltext));
#endif
	if ((bytes_read > 2) && (bytes_read == raw_frame[2] + 3)) { // add 3 to payload length at byte 2 to account for sync/destination/checksum packet 
		if (raw_frame[0] == static_cast<uint8_t>(JVS_OPCODES_SYNC)) {
			if (raw_frame[1] == 0) { // destination will always be 0
				if (raw_frame[3] != JVS_OPCODES_OK || raw_frame[4] != JVS_OPCODES_OK) { // check for OK opcodes
					LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - jdata[0] JVS NOT OK!\r\n"));
					return false;
				}

				int checksum = raw_frame[1] + raw_frame[2];
				int received_size = raw_frame[2];

				if (received_size > size) {
					if (log_error_)
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Payload too large for buffer!\r\n"));
					return false;
				}
				int i = 3;
				int j = 0;
				bool escape_code = false;

				for (i = 3; i < bytes_read - 1; ++i) { // confirm checksum and do conversions for escape codes
					if (raw_frame[i] == static_cast<uint8_t>(JVS_OPCODES_ESCAPE))
						escape_code = true;
					else if (escape_code) {
						buffer[j] = raw_frame[i] + 1;
						checksum += raw_frame[i] + 1;
						escape_code = false;
						++j;
					}
					else {
						buffer[j] = raw_frame[i];
						checksum += raw_frame[i];
						++j;
					}
				}
				int received_checksum = raw_frame[i];
				++j;

				if (j != received_size) {
					if (log_error_)
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Incorrect payload length received!\r\n"));
					return false;
				}

				if (checksum % 256 != received_checksum) {
					if (log_error_) {
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Bad checksum! %02x != %02x\r\n", checksum % 256, received_checksum));
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Raw frame:\r\n\t"));
						memset(ltext, 0, sizeof(ltext));

						for (int i = 0; i < bytes_read; ++i) {
							wchar_t debug_buffer[16] = { 0 };
							swprintf_s(debug_buffer, sizeof(debug_buffer), L"[%02x]", raw_frame[i]);
							wcscat_s(ltext, sizeof(ltext), debug_buffer);
						}
						wcscat_s(ltext, sizeof(ltext), L"\r\n");
						LogEvent(log_file_handle_, ltext, wcslen(ltext));

						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Payload:\r\n\t"));
						memset(ltext, 0, sizeof(ltext));

						for (int i = 0; i < received_size; ++i) {
							wchar_t debug_buffer[16] = { 0 };
							swprintf_s(debug_buffer, sizeof(debug_buffer), L"[%02x]", buffer[i]);
							wcscat_s(ltext, sizeof(ltext), debug_buffer);
						}
						wcscat_s(ltext, sizeof(ltext), L"\r\n");
						LogEvent(log_file_handle_, ltext, wcslen(ltext));
					}
					return false;
				}
#ifdef DEBUG
				LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Payload:\r\n\t"));
				memset(ltext, 0, sizeof(ltext));

				for (auto i = 0; i < received_size; ++i) {
					wchar_t debug_buffer[16] = { 0 };
					swprintf_s(debug_buffer, sizeof(debug_buffer), L"[%02x]", buffer[i]);
					wcscat_s(ltext, sizeof(ltext), debug_buffer);
				}
				wcscat_s(ltext, sizeof(ltext), L"\r\n");
				LogEvent(log_file_handle_, ltext, wcslen(ltext));
#endif
				return true;
			}
			if (log_error_)
				LogEvent(log_file_handle_, ltext, swprintf_s(ltext, 256, L"ReadJVS - Wrong packet destination!\r\n"));
			return false;
		}
		if (log_error_)
			LogEvent(log_file_handle_, ltext, swprintf_s(ltext, 256, L"ReadJVS - Corrupt sync frame!\r\n"));
		return false;
	}
	if (log_error_)
		LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"ReadJVS - Incomplete frame received!\r\n"));
	return false;
}


bool Input::WriteJVS(uint8_t dest, const uint8_t* data, int size) {
	wchar_t ltext[256] = { 0 };
	uint8_t raw_frame[256] = { 0 };

	OVERLAPPED os = { 0 };
	os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	raw_frame[0] = static_cast<uint8_t>(JVS_OPCODES_SYNC);
	raw_frame[1] = dest;

	int j = 3;
	int checksum = dest;

	for (auto i = 0; i < size; ++i) {
		if (j > sizeof(raw_frame)) {
			if (log_error_)
				LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"WriteJVS - Payload too large for buffer!\r\n"));
			return false;
		}
		if (data[i] == static_cast<uint8_t>(JVS_OPCODES_SYNC) || data[i] == static_cast<uint8_t>(JVS_OPCODES_ESCAPE)) {
			raw_frame[j] = static_cast<uint8_t>(JVS_OPCODES_ESCAPE);
			raw_frame[j + 1] = data[i] - 1;
			checksum += data[i];
			j += 2;
		}
		else {
			raw_frame[j] = data[i];
			checksum += data[i];
			++j;
		}
	}
	raw_frame[2] = static_cast<uint8_t>(j - 2);
	raw_frame[j] = static_cast<uint8_t>((checksum + raw_frame[2]) % 256);
	++j;

#ifdef DEBUG
	LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"WriteJVS - Raw frame:\r\n\t"));
	memset(ltext, 0, sizeof(ltext));

	for (auto i = 0; i < j; ++i) {
		wchar_t debug_buffer[16] = { 0 };
		swprintf_s(debug_buffer, sizeof(debug_buffer), L"[%02x]", raw_frame[i]);
		wcscat_s(ltext, sizeof(ltext), debug_buffer);
	}
	wcscat_s(ltext, sizeof(ltext), L"\r\n");
	LogEvent(log_file_handle_, ltext, wcslen(ltext));
#endif
	DWORD bytes_sent = 0;

	if (!WriteFile(com_port_handle_, &raw_frame, j, &bytes_sent, &os)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			if (log_error_)
				LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"WriteJVS - Write Error!\r\n"));
			CloseHandle(os.hEvent);
			return false;
		}
		DWORD result = WaitForSingleObject(os.hEvent, kReadTimeout);
		switch (result) {
			case WAIT_TIMEOUT: {
				if (log_error_)
					LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"WriteJVS - Timeout exceeded!\r\n"));
				CloseHandle(os.hEvent);
				return false;
				break;
			}
			case WAIT_OBJECT_0: {
				if (!GetOverlappedResult(com_port_handle_, &os, &bytes_sent, false)) {
					if (log_error_)
						LogEvent(log_file_handle_, ltext, swprintf_s(ltext, sizeof(ltext), L"WriteJVS - Write Error!\r\n"));
					CloseHandle(os.hEvent);
					return false;
				}
				break;
			}
		}
	}
	CloseHandle(os.hEvent);
	return true;
}

bool Input::CheckTest(uint16_t& player_bits, uint8_t& input) {
	// was test button pressed?
	// return value = true if yes, false if no

	bool test_pressed = ((input & kJvsButtons[kTestButton]) > 0);
	player_bits = (player_bits & static_cast<uint16_t>(~1)) | (-static_cast<uint16_t>(test_pressed) & static_cast<uint16_t>(1));

	return test_pressed;
}

bool Input::CheckButtons(uint16_t& player_bits, uint16_t& input) {
	// pass raw JVS data and return output required to trigger
	// return value = true if any button pressed, false if none

	uint16_t current_flags = player_bits;

	// preserve the current test button state since we handle it separately
	player_bits = (player_bits & (uint16_t)~(1 << kTestButton)) | (-((current_flags & (1 << kTestButton)) > 0) & (uint16_t)(1 << kTestButton));
	for (int i = kStartButton; i < kNumButtons; ++i)
		player_bits = (player_bits & static_cast<uint16_t>(~(1 << i))) | (-static_cast<uint16_t>(((input & kJvsButtons[i])) > 0) & static_cast<uint16_t>(1 << i));

	if (!player_bits)
		return false;
	return true;
}

bool Input::ReadButtons() {
	uint8_t jdata[256] = { 0 };
	wchar_t ltext[256] = { 0 };

	for (auto node = 0; node < kNumNodes; ++node) {
		bool key_pressed = false;
		jkey_[node].poll_previous = jkey_[node].poll_now;

		// throttle our polling rate so we're not trying to read too fast
		do {
			QueryPerformanceCounter(&jkey_[node].poll_now);
		} while (TimeDifference(jkey_[node].poll_previous, jkey_[node].poll_now, jkey_[node].frequency) < ((1.0 / static_cast<double>(kPollingRate) * 1000.0)));

		jkey_[node].idle_now = jkey_[node].poll_now;

		if (!WriteJVS(node + 1, kInputRequest[node], 3))
			return false;

		Sleep(5);

		if (!ReadJVS(jdata, sizeof(jdata)))
			return false;

		// store the inputs from the previous run
		for (auto player = 0; player < kNumPlayersPerNode; ++player)
			jkey_[node].players_previous[player] = jkey_[node].players[player];

		// only check the first player for test since there's only 1 test button
		key_pressed = CheckTest(jkey_[node].players[0].bits, jdata[2]);

		// check other butans
		for (auto player = 0; player < kNumPlayersPerNode; ++player) {
			uint16_t input = static_cast<uint16_t>(jdata[3 + (player << 1)]) + static_cast<uint16_t>(jdata[4 + (player << 1)] << 8);
			key_pressed = CheckButtons(jkey_[node].players[player].bits, input);
		}

		if (key_pressed)
			jkey_[node].idle_start = jkey_[node].idle_now;
	}
	return true;
}

std::vector<std::vector<uint16_t>> Input::get_players() {
	std::vector<std::vector<uint16_t>> players;
	for (auto node = 0; node < kNumNodes; ++node) {
		std::vector<uint16_t> player_bits;
		for (auto player = 0; player < kNumPlayersPerNode; ++player)
			player_bits.emplace_back(jkey_[node].players[player].bits);
		players.emplace_back(player_bits);
	}
	return players;
}

std::vector<std::vector<uint16_t>> Input::get_players_previous() {
	std::vector<std::vector<uint16_t>> players;
	for (auto node = 0; node < kNumNodes; ++node) {
		std::vector<uint16_t> player_bits;
		for (auto player = 0; player < kNumPlayersPerNode; ++player)
			player_bits.emplace_back(jkey_[node].players_previous[player].bits);
		players.emplace_back(player_bits);
	}
	return players;
}

bool Input::IdleTimeout() {
	int timeout_count = 0;

	for (auto i = 0; i < kNumNodes; ++i) {
		if (TimeDifference(jkey_[i].idle_start, jkey_[i].idle_now, jkey_[i].frequency) > static_cast<double>(kIdleTimeout))
			++timeout_count;
	}
	if (timeout_count == kNumNodes)
		return false;
	return true;
}

}
