#ifndef GWENJVS_HELPERS_H
#define GWENJVS_HELPERS_H

#include <cstdio>
#include <cstdint>
#include <string>
#include <windows.h>

// helper functions and defines
#define W(x)          W_(x)
#define W_(x)         L ## x
#define N(x)          x
#define STR(x, t)     STR_(x, t)
#define STR_(x, t)    t(#x)

#define LOCATION_(t)  t(__FILE__) t("(") STR(__LINE__, t) t(")")
#define LOCATION      LOCATION_(N)
#define WLOCATION     LOCATION_(W)

#define PRINT_ERROR(fmt, ...) printf("[error] " LOCATION ": " fmt "\n", __VA_ARGS__)
#define PRINT_WARN(fmt, ...) printf("[warn] " fmt "\n", __VA_ARGS__)
#define PRINT_INFO(fmt, ...) printf("[info] " fmt "\n", __VA_ARGS__)

#define WCSTR(x) GwenJVS::widen(x).c_str()
#define NCSTR(x) GwenJVS::narrow(x).c_str()

namespace GwenJVS {

// enum for index of keymaps
enum kButtons {
	kTestButton,
	kStartButton,
	kServiceButton,
	kUpStick,
	kDownStick,
	kLeftStick,
	kRightStick,
	kButton1,
	kButton2,
	kButton3,
	kButton4,
	kButton5,
	kButton6
};

std::string narrow(const wchar_t* s);
std::string narrow(const std::wstring& s);
std::wstring widen(const char* s);
std::wstring widen(const std::string& s);

std::string strtoupper(const char* s);
std::string strtoupper(const std::string& s);
std::string strtolower(const char* s);
std::string strtolower(const std::string& s);

// inline functions
double inline TimeDifference(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER frequency) {
	return (static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0) / static_cast<double>(frequency.QuadPart);
}

void inline LogEvent(HANDLE log_file, const wchar_t text[], size_t size) {
	DWORD lwrote = 0;
	auto output = narrow(text);
	WriteFile(log_file, output.c_str(), output.length(), &lwrote, NULL);
	FlushFileBuffers(log_file);

	if (IsDebuggerPresent()) {
		OutputDebugStringW(text);
	}
}

} // namespace GwenJVS

#endif

