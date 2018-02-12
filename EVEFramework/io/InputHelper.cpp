#include <iostream>
#include <sstream>
#include <string>
#include "InputHelper.h"
#include "screenpos.h"

InputHelper::InputHelper(const std::atomic_bool &ref) : stop(ref) {
	log = spdlog::get("log");
}

//!!!!!!!!!! DON'T FORGET TO DELETE :^) !!!!!!!!!!!
tagINPUT* getInputArray(int size) {
	INPUT* ip = new INPUT[size];
	for (int i = 0; i < size; i++) {
		ip[i].type = INPUT_KEYBOARD;
		ip[i].ki.wScan = 0;
		ip[i].ki.time = 0;
		ip[i].ki.dwExtraInfo = 0;
	}
	return ip;
}

//Send String input via keyboard
int InputHelper::kbdinput(std::string input)
{
	log->debug("Sending input: " + input);
	// Create a generic keyboard event structure
	INPUT ips;
	INPUT* ip = getInputArray(2);

	ips.type = INPUT_KEYBOARD;
	ips.ki.wScan = 0;
	ips.ki.time = 0;
	ips.ki.dwExtraInfo = 0;

	for (unsigned int i = 0; i < input.size(); i++)
	{
		char in = std::toupper(input.at(i));

		if (isupper(input.at(i))) {
			// Press the "Shift" key
			ips.ki.wVk = VK_SHIFT;
			ips.ki.dwFlags = 0; // 0 for key press
			SendInput(1, &ips, sizeof(INPUT));
			Sleep(20);
		}

		// Press key
		ip[0].ki.wVk = in;
		ip[0].ki.dwFlags = 0; // 0 for key press
		// Release key
		ip[1].ki.wVk = in;
		ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(2, ip, sizeof(INPUT));

		if (isupper(input.at(i))) {
			// Release the "Shift" key
			ips.ki.wVk = VK_SHIFT;
			ips.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &ips, sizeof(INPUT));
		}

		TSleep(90); // pause
	}
	
	delete[] ip;
	return 0;
}

//Send button-press input via keyboard
int InputHelper::kbdinput(int btn)
{
	std::stringstream ss;
	ss << "Sending input: VK_0x" << std::hex << btn;
	log->debug(ss.str());
	// Create a generic keyboard event structure
	INPUT* ip = getInputArray(2);

	// Press button
	ip[0].ki.wVk = btn;
	ip[0].ki.dwFlags = 0; // 0 for key press
	// Release button
	ip[1].ki.wVk = btn;
	ip[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, ip, sizeof(INPUT));
	TSleep(100); // pause

	delete[] ip;
	return 0;
}

//Send ctrl+c via keyboard
void InputHelper::kbdcopy()
{
	log->debug("Sending input: CTRL+C");
	// Create a generic keyboard event structure
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press button
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));
	Sleep(20);

	// Press button
	ip.ki.wVk = 'C';
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release button
	ip.ki.wVk = 'C';
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
	Sleep(20);

	// Release button
	ip.ki.wVk = VK_CONTROL;
	ip.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
	TSleep(100);

	return;
}

//Click x/y position with mouse
//Needs window parameter
void InputHelper::mouseLeftClick(const int &x, const int &y, const HWND &hWindow)
{
	// string for log
	std::stringstream ss;
	ss << "Click(" << x << ", " << y << ")";
	log->debug(ss.str());

	// offset for Windows window frame 9/32
	int a = x + 9;
	int b = y + 32;

	// get the window position
	RECT rect;
	GetWindowRect(hWindow, &rect);

	// calculate scale factor
	const double XSCALEFACTOR = 65535.0f / (GetSystemMetrics(SM_CXSCREEN));
	const double YSCALEFACTOR = 65535.0f / (GetSystemMetrics(SM_CYSCREEN));

	// calculate target position relative to application
	double nx = (a + rect.left) * XSCALEFACTOR;
	double ny = (b + rect.top) * YSCALEFACTOR;

	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;

	Input.mi.dx = (LONG)nx;
	Input.mi.dy = (LONG)ny;

	// set move cursor directly and left click
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	SendInput(1, &Input, sizeof(INPUT));
	Sleep(30);
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
	Sleep(10);
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &Input, sizeof(INPUT));
	POINT p;
	mousePosOutput(p, hWindow);
	TSleep(250);
}

//right click current position
void InputHelper::mouseRightClick(const HWND &hWindow)
{
	log->debug("RClick");

	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;

	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
	Sleep(10);
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1, &Input, sizeof(INPUT));
	POINT p;
	mousePosOutput(p, hWindow);
	TSleep(250);
}

void InputHelper::mousePosOutput(POINT &p, const HWND &hwnd)
{
	if (GetCursorPos(&p))
	{
		if (ScreenToClient(hwnd, &p))
		{
			std::stringstream ss;
			ss << "RegisteredClick(" << p.x << ", " << p.y << ")";
			log->debug(ss.str());
		}
	}
}

//splits the string that was copied from the tree
//into individual words for future analysys
//returns empty list if it fails
std::vector<std::string> InputHelper::listStringSplit(const std::string &input) {
	std::vector<std::string> list;
	std::string buffer;
	char prev = NULL;
	std::stringstream ss(input);

	while (std::getline(ss, buffer, '\n')) {
		list.push_back(buffer);
	}
	return list;
}

//returns line index; -1 for failure
int InputHelper::listSelect(const HWND &hWindow, const std::string &searchString, 
	const int &listItemX, const int &listItemY, const int &listGap) {
	
	std::string clipboard;
	int numberChecks = 0;
	mouseLeftClick(listItemX, listItemY, hWindow);

	//as long as item is not in tree
	while (true) {

		//get clipboard contents
		TSleep(300);
		kbdcopy();
		if (OpenClipboard(NULL) != FALSE)
		{
			HANDLE clip = GetClipboardData(CF_TEXT);
			if (clip != NULL)
			{
				clipboard = reinterpret_cast<char*>(GlobalLock(clip));
				GlobalUnlock(clip);
			}
			CloseClipboard();
		}
		//abort if clipboard empty
		if (clipboard.empty()) {
			log->error("Could not copy list contents!");
			return -1;
		}
		//split copied string
		std::vector<std::string> stringlist = listStringSplit(clipboard);
		if (stringlist.empty()) {
			log->error("Selected list empty!");
			return -1;
		}
		//if searchString was found in list, select
		int strcount = 0;
		for (std::string str : stringlist) {
			str = str.substr(0, str.find("\t", 0));
			if (str == searchString) {
				log->debug("Item found in list at line " + std::to_string(strcount + numberChecks));
				log->debug("Selecting...");
				return strcount + numberChecks;
			}
			strcount++;
		}
		//expand next in list
		mouseLeftClick(listItemX, listItemY + (numberChecks * listGap), hWindow);

		numberChecks++;
		if (numberChecks > 17) {

			//TODO: try tab select as last resort
			log->error("Error while searching list: OOB");
			break;
		}
	}
	log->critical("EFA5478: Uh oh.... You should not end up here....");
	return -1;
}