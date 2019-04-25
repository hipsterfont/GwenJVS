#include "gwenjvsfaker.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	std::string com_port = GwenJVS::kDefaultPort;
	std::string log_location = GwenJVS::kLogLocation;
	int output_mode = kOutputKb;
	void* output = nullptr;
	bool log_error = true;
	wchar_t ltext[256] = { 0 };
	uint8_t jvsdata[256] = { 0 };

	if (__argc > 1) {
		for (auto i = 1; i < __argc; ++i) {
			std::string cmdLine = NCSTR(__wargv[i]);
			
			if (cmdLine[0] == '-') {
				if (toupper(cmdLine[1]) == 'K')
					output_mode = kOutputKb;
				if (toupper(cmdLine[1]) == 'X')
					output_mode = kOutputXInput;
				if (toupper(cmdLine[1]) == 'D')
					output_mode = kOutputDInput;
				if (toupper(cmdLine[1]) == 'L')
					log_error = true;
				if (toupper(cmdLine[1]) == 'C') {
					if ((i + 1) < __argc) {
						com_port = GwenJVS::strtoupper(NCSTR(__wargv[i + 1]));
						++i;
					}
				}
			}
		}
	}

	if (!CreateDirectoryW(GwenJVS::widen(log_location).c_str(), NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			return 1;
	}

	auto log_file_handle = CreateFileW(GwenJVS::widen(log_location + std::string("\\gwenjvs.log")).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	auto com_port_handle = CreateFileW(GwenJVS::widen(std::string("\\\\.\\") + com_port).c_str() , GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (com_port_handle == INVALID_HANDLE_VALUE) {
		GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"Main - Could not open COM port! Error: %d.\r\n", GetLastError()));
		return 1;
	}

	auto jvs_input = std::make_unique<GwenJVS::Input>(com_port_handle, log_file_handle, log_error);

/*	switch (output_mode) {
		case kOutputXInput: { 
			output = new GwenJVS::XOutput();
			reinterpret_cast<GwenJVS::XOutput*>(output)->Init();
			break;
		}
		case kOutputDInput: { 
			output = new GwenJVS::DOutput();
			reinterpret_cast<GwenJVS::DOutput*>(output)->Init();
			break;
		}
		case kOutputKb:
		default: {
			output = new GwenJVS::KeyboardOutput();
			reinterpret_cast<GwenJVS::KeyboardOutput*>(output)->Init();
			break;
		}
	}

	GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"Main - COM Setup.\r\n"));
	jvs_input->Initialize();
*/
	while (true) {
		while (true) {
			/* if (!jvs_input->IdleTimeout()) {
				GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"I failed at jvs_input->IdleTimeout().\r\n"));
				break;
			} */

			if (!jvs_input->ReadJVS(jvsdata, sizeof(jvsdata))) {
				GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"I failed at jvs_input->ReadJVS().\r\n"));
				break;
			}

			/*
			if (output_mode == kOutputKb)
				if (!reinterpret_cast<GwenJVS::KeyboardOutput*>(output)->PressButtons(jvs_input->get_players(), jvs_input->get_players_previous())) {
					GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"I failed at reinterpret_cast<GwenJVS::KeyboardOutput*>(output)->PressButtons().\r\n"));
					break;
				}

			if (output_mode == kOutputXInput)
				if (!reinterpret_cast<GwenJVS::XOutput*>(output)->PressButtons(jvs_input->get_players(), jvs_input->get_players_previous())) {
					GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"I failed at reinterpret_cast<GwenJVS::XOutput*>(output)->PressButtons().\r\n"));
					break;
				}

			if (output_mode == kOutputDInput)
				if (!reinterpret_cast<GwenJVS::DOutput*>(output)->PressButtons(jvs_input->get_players(), jvs_input->get_players_previous())) {
					GwenJVS::LogEvent(log_file_handle, ltext, swprintf_s(ltext, sizeof(ltext), L"I failed at reinterpret_cast<GwenJVS::DOutput*>(output)->PressButtons().\r\n"));
					break;
				}
			*/
		}
	}

	return 0;
}