#pragma once
#include "..\main.h"

struct InputHelper {
	std::shared_ptr<spdlog::logger> log;
	const std::atomic_bool &stop;

	InputHelper(const std::atomic_bool &stop);
	int kbdinput(std::string input);
	int kbdinput(int btn);
	void kbdcopy();
	void mouseLeftClick(const int &x, const int &y, const HWND &hWindow);
	void mouseRightClick(const HWND &hWindow);
	void mousePosOutput(POINT &p, const HWND &hwnd);
	int listSelect(const HWND &hWindow, const std::string &searchString,
		const int &listItemX, const int &listItemY, const int &listGap);
	std::vector<std::string> listStringSplit(const std::string &input);

};