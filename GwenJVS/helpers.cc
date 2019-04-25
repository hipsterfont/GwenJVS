// helper functions

#include "helpers.h"

namespace GwenJVS {

std::string narrow(const wchar_t* s) {
	if (wcslen(s) == 0)
		return std::string();

	// get buffer needed
	auto bufNeed = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	auto returnCStr = new char[bufNeed];

	// convert to utf-8
	WideCharToMultiByte(CP_UTF8, 0, s, -1, returnCStr, bufNeed, NULL, NULL);

	auto returnStr = std::string(returnCStr);
	delete[] returnCStr;

	return returnStr;
}

std::wstring widen(const char* s) {
	if (strlen(s) == 0)
		return std::wstring();

	// get buffer needed
	auto bufNeed = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	auto returnCStr = new wchar_t[bufNeed];

	// convert to widechar
	MultiByteToWideChar(CP_UTF8, 0, s, -1, returnCStr, bufNeed);

	auto returnStr = std::wstring(returnCStr);
	delete[] returnCStr;

	return returnStr;
}

std::string narrow(const std::wstring& s) {
	if (wcslen(s.c_str()) == 0)
		return std::string();

	// get buffer needed
	auto bufNeed = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, NULL, NULL);
	auto returnCStr = new char[bufNeed];

	// convert to utf-8
	WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, returnCStr, bufNeed, NULL, NULL);

	auto returnStr = std::string(returnCStr);
	delete[] returnCStr;

	return returnStr;
}

std::wstring widen(const std::string& s) {
	if (strlen(s.c_str()) == 0)
		return std::wstring();

	// get buffer needed
	auto bufNeed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
	auto returnCStr = new wchar_t[bufNeed];

	// convert to widechar
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, returnCStr, bufNeed);

	auto returnStr = std::wstring(returnCStr);
	delete[] returnCStr;

	return returnStr;
}

std::string strtoupper(const char* s) {
	auto len = strlen(s);
	std::string output;

	for (auto i = 0; i < len; ++i)
		output += toupper((*(s + i)));

	return output;
}

std::string strtoupper(const std::string& s) {
	auto len = strlen(s.c_str());
	std::string output;

	for (auto i = 0; i < len; ++i)
		output += toupper(s[i]);

	return output;
}

std::string strtolower(const char* s) {
	auto len = strlen(s);
	std::string output;

	for (auto i = 0; i < len; ++i)
		output += tolower((*(s + i)));

	return output;
}

std::string strtolower(const std::string& s) {
	auto len = strlen(s.c_str());
	std::string output;

	for (auto i = 0; i < len; ++i)
		output += tolower(s[i]);

	return output;
}

} // namespace GwenJVS