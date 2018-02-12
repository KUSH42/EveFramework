#define NOMINMAX
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <vector>
#include <iomanip>
#include <thread>
#include <ctype.h>
#include <locale>
#include <memory>
#include <vector>
#include <atomic>
#include <chrono>
#include "main.h"
#include "spdlog\sinks\dist_sink.h"
#include "spdlog\sinks\wincolor_sink.h"
#include "logUtils\LogUtil.h"
#include "Order.h"
#include "io\InputHelper.h"
#include "io\screenpos.h"

//TODO: Remove absolute path
#define LOGPATH "%USERPROFILE%\\Documents\\EVE\\logs\\Marketlogs\\Domain-Hobgoblin II-2017.04.17 162717.txt"
#define DEBUGLEVEL debug

std::atomic<bool> stop = false;
std::atomic<bool> finish = false;
void TSleep(long ms);
void mousePosOutputThread(POINT p, const HWND hwnd);
void initLogger();
void marketScan(const HWND &hwnd);
void sellRoutine(const HWND &hWindow);

int main(int *argc, char *argv[]) {
	initLogger();
	auto log = spd::get("log");

	std::cout << "Welcome to the EVE Online Market Analysis Tool (v0.001)" << std::endl << std::endl;

	log->info("Starting test in 3...");
	TSleep(1200);
	log->info("Starting test in 2...");
	TSleep(1200);
	log->info("Starting test in 1...");
	TSleep(1200);
	log->info("Starting test!");
	TSleep(1500);

	// find window
	HWND hWindow = FindWindow("triuiScreen", NULL);
	InputHelper ih(&stop);
	if (NULL != hWindow) {

		POINT p;
		std::thread t1(mousePosOutputThread, p, hWindow);

		//entry
		if (SetForegroundWindow(hWindow)) {
			//std::thread t2(marketScan, hWindow);
			//std::thread t2(sellRoutine, hWindow);
		}
		else {
			log->error("Could not set application to foreground.");
		}

	}
	else {
		log->error("Could not find target window.");
	}

	//ctrl+p for pause
	while (!finish) {
		if (GetAsyncKeyState(VK_PAUSE)) {
			log->info("---PAUSED---");
			stop = true;
			Sleep(200);
			while (stop) {
				if (GetAsyncKeyState(VK_PAUSE)) {
					stop = false;
					log->info("---UNPAUSED---");
				}
				Sleep(40);
			}
		}
		Sleep(400);
	}
	stop = false;
	//t2.join();
	std::cin.get();
}

void sellRoutine(const HWND &hWindow) {
	auto log = spd::get("log");
	log->info("Starting sell routine...");

	LogUtil logUtil;
	std::vector<std::string> watchlist;
	//TODO: Remove absolute path
	logUtil.watchlistImport("%USERPROFILE%\\Documents\\Visual Studio 2017\\Projects\\ConsoleApplication1\\ConsoleApplication1\\sale.txt", watchlist);
	InputHelper in(stop);
	in.mouseLeftClick(TASKBAR_INVENTORY, hWindow);
	TSleep(1000);
	in.mouseLeftClick(INVENTORY_STATIONINVENTORY, hWindow);
	TSleep(1000);
	in.mouseLeftClick(INVENTORY_LISTVIEW, hWindow);
	TSleep(1000);
	std::string iteminfo;
	for (std::string searchString : watchlist) {
		in.mouseLeftClick(INVENTORY_SEARCHBOX_CLEAR, hWindow);
		TSleep(1000);
		in.kbdinput(searchString);
		TSleep(1000);
		in.kbdinput(VK_RETURN);
		TSleep(1000);
		in.mouseLeftClick(INVENTORY_FIRSTITEM, hWindow);
		TSleep(1000);
		if (in.listSelect(hWindow, searchString, INVENTORY_FIRSTITEM, INVENTORY_ITEM_GAP) < 0) {
			log->info("Item {1} not found in Inventory. Proceeding with next entry.", searchString);
			break;
		}
		TSleep(1000);
		in.mouseRightClick(hWindow);
		TSleep(1000);
		//get cursor pos because we don't know which line we're at right now
		POINT p;
		int offset;
		if (GetCursorPos(&p))
		{
			if (ScreenToClient(hWindow, &p))
			{
				offset = p.y - INVENTORY_FIRSTITEMY;
			}
		}
		in.mouseLeftClick(INVENTORY_RCLICK_SELL + offset, hWindow);
		log->info("Done.", searchString);
	}
	log->info("------End------");
	finish = true;
}

void marketScan(const HWND &hWindow) {
	auto log = spd::get("log");

	LogUtil logUtil;
	std::vector<std::string> watchlist;
	logUtil.watchlistImport("C:\\Users\\Kush\\Documents\\Visual Studio 2017\\Projects\\ConsoleApplication1\\ConsoleApplication1\\minerals.txt", watchlist);
	InputHelper in(stop);
	in.mouseLeftClick(TASKBAR_MARKET, hWindow);
	TSleep(2000);
	for (std::string searchString : watchlist) {
		log->info("Looking for: " + searchString);
		in.mouseLeftClick(MARKET_SEARCHBOX_CLEAR, hWindow);
		TSleep(1500);
		in.kbdinput(searchString);
		TSleep(500);
		in.kbdinput(VK_RETURN);
		TSleep(1000);
		in.kbdinput(VK_TAB);
		TSleep(200);
		in.kbdinput(VK_TAB);
		TSleep(200);
		log->debug("Exploring search tree!");
		int line = in.listSelect(hWindow, searchString, MARKET_TREE_FIRSTITEM, MARKET_TREE_NEXT_GAP);
		if (line < 0) {
			log->error("Error navigating tree!");
		}
		else {
			in.mouseLeftClick(MARKET_TREE_FIRSTITEM + (line * MARKET_TREE_NEXT_GAP), hWindow);
			TSleep(1500);
			in.mouseLeftClick(MARKET_ITEM_EXPORT_LOG, hWindow);
			log->info("Exported: " + searchString);
			std::thread t3(&LogUtil::importAnalysis, logUtil, searchString);
			TSleep(1500);
			in.mouseLeftClick(LOGEXPORT_POPUP_OK, hWindow);
			TSleep(100);
			t3.join();
		}
	}
	log->info("------End------");
	spd::drop_all();
	finish = true;
}

void mousePosOutputThread(POINT p, HWND hwnd)
{
	auto log = spd::stdout_color_mt("mousePos");
	log->info("Started mouse output thread");
	while (true) {
		//Check if right mouse button is pressed
		if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
		{
			if (GetCursorPos(&p))
			{
				if (ScreenToClient(hwnd, &p))
				{
					std::string buffer = std::to_string(p.x);
					buffer.append(", ");
					buffer.append(std::to_string(p.y));
					log->info(buffer);
				}
			}
		}
		Sleep(100);
	}
}

void initLogger() {
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	auto dist_sink = std::make_shared<spd::sinks::dist_sink_mt>();
	auto sink_console = std::make_shared<spd::sinks::stdout_sink_mt>();
	auto sink1 = std::make_shared<spd::sinks::wincolor_stdout_sink_mt>();
	auto sink2 = std::make_shared<spd::sinks::simple_file_sink_mt>("eve.log");

	dist_sink->add_sink(sink1);
	dist_sink->add_sink(sink2);

	auto logger = std::make_shared<spd::logger>("log", dist_sink);
	spdlog::register_logger(logger);
	logger->set_level(spd::level::DEBUGLEVEL);
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	logger->debug("Logger initiated within {:d} microseconds ({:03.2f}ms)", duration, (double) duration/1000);
}

void TSleep(long ms) {
	while (stop) {
		Sleep(ms);
	}

	Sleep(ms);
}