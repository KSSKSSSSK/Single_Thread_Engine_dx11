#pragma once
#include <unordered_map>
#include <Windows.h>


class WindowsMessageMap
{
private:
	std::unordered_map<DWORD, std::wstring> map;
public:
	std::wstring operator() (DWORD msg, LPARAM lp, WPARAM wp) const;
public:
	WindowsMessageMap();
};