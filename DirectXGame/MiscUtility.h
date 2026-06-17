#pragma once
#include <string>

std::wstring ConvertWString(const std::string& str);

std::string ConvertString(const std::wstring& str);
