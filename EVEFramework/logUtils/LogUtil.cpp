#include <fstream>
#include <ctime>
#include <sstream>
#include <iostream>
#include <ctype.h>
#include <algorithm> 
#include <iomanip>
#include "..\Order.h"
#include "..\io\InputHelper.h"
#include "LogUtil.h"
#include "..\main.h"
#define LOGPATHL L"%USERPROFILE%\\Documents\\EVE\\logs\\Marketlogs\\*.txt"
#define LOGPATH "%USERPROFILE%\\Documents\\EVE\\logs\\Marketlogs\\"

std::string STATIONID = "60008494";
bool finddate(const std::string &str);

LogUtil::LogUtil() {
	log = spd::get("log");
}

std::vector<Order> LogUtil::importAnalysis(const std::string &searchString) {

	log->debug("Starting log analysis");
	std::vector<Order> orders;

	//get all files from folder
	WIN32_FIND_DATA search_data;
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile(LOGPATH, &search_data);
	std::vector<std::string> loglist;
	std::string searchStringDelim = "-";
	searchStringDelim.append(searchString);
	searchStringDelim.append("-");
	while (handle != INVALID_HANDLE_VALUE)
	{
		//check if logname contains search item
		std::string str(search_data.cFileName);
		if (str.find(searchStringDelim) != std::string::npos) {
			loglist.push_back(str);
		}
		if (FindNextFile(handle, &search_data) == FALSE)
			break;
	}
	//Close the handle after use or memory/resource leak
	FindClose(handle);
	//check for latest log
	//get date string, check date, discard
	loglist.erase(std::remove_if(loglist.begin(), loglist.end(), finddate), loglist.end());
	//check for highest time
	long time = 0;
	for (std::string str : loglist) {
		int len = str.length();
		std::string buffer = str.substr(len - 10, len - 6);
		long tbuf = stol(buffer);
		if (tbuf > time) {
			time = tbuf;
		}
	}
	//pick correct logname
	std::string logname;
	for (std::string str : loglist) {
		if (str.find(std::to_string(time)) != std::string::npos) {
			logname = str;
			break;
		}
	}
	log->debug("Logname: " + logname);

	//open log
	std::vector<std::vector<std::string>> v_orders;
	if (!marketImport(LOGPATH + logname, v_orders))
	{
		double maxBuy = 0;
		double minSell = std::numeric_limits<double>::max();
		int buyOrders = 0;
		for (unsigned int x = 0; x < (v_orders.size() - 1); x++)
		{
			bool sameStationFlag = (v_orders[x][10].find(STATIONID) != std::string::npos);
			std::string t = "True";
			double price = atof(v_orders[x][0].c_str());
			if (v_orders[x][7] == t)
			{
				if (sameStationFlag) {
					if (price > maxBuy)
					{
						maxBuy = price;
					}
				}
				buyOrders++;
			}
			else {
				if (sameStationFlag) {
					if (price < minSell)
					{
						minSell = price;
					}
				}
			}
		}

		std::cout << "\n\t" << searchString << std::endl;
		std::cout << "Number of buy orders: " << buyOrders << std::endl;
		std::cout << "Number of sell orders: " << v_orders.size() - buyOrders << std::endl;
		std::cout << std::fixed << std::setprecision(2);
		std::cout << "Min sell price: " << minSell << " ISK\n";
		std::cout << "Max buy price: " << maxBuy << " ISK\n";
		std::cout << "Profit Margin:  " << (minSell / maxBuy - 1) * 100 << "%\n\n\n";

		for (unsigned int i = 0; i < v_orders.size(); i++) {
			Order order(v_orders[0]);
			orders.push_back(order);
		}
		std::string logmsg = "Extracted log containing ";
		logmsg.append(std::to_string(orders.size()));
		logmsg.append(" orders");
		log->debug(logmsg);
		return orders;
	}
	else {
		log->error("FATAL ERROR! Could not open file... Terminating...");
	}
	log->debug("Log analysis finished");
	return orders;
}

int LogUtil::marketImport(const std::string &filepath, std::vector<std::vector<std::string>> &p_v_strings) {

	std::ifstream logfile(filepath);

	if (logfile.is_open()) {
		size_t pos = 0;
		std::string line;
		std::string delimiter = ",";

		//skip first line containing column info
		getline(logfile, line);
		while (getline(logfile, line))
		{
			//split string into individual values
			int i = 0;
			std::vector<std::string> buffer;
			while ((pos = line.find(delimiter)) != std::string::npos) {
				buffer.push_back(line.substr(0, pos));
				line.erase(0, pos + delimiter.length());
			}
			p_v_strings.push_back(buffer);
		}
		logfile.close();
	}
	else {
		log->error("Failed to import market log from file");
		return 1;
	}
	log->info("Imported market log from file");
	return 0;
}

//imports strings for watchlist items
//provide filepath and string vector, receive vector of item strings
//returns 0 for success, 1 if failed to open file
int LogUtil::watchlistImport(const std::string &filepath, std::vector<std::string> &p_v_strings) {

	std::ifstream logfile(filepath);

	if (logfile.is_open()) {
		size_t pos = 0;
		std::string line;

		while (getline(logfile, line))
		{
			p_v_strings.push_back(line);
		}
		logfile.close();
	}
	else {
		log->error("Failed to import watchlist items from file");
		return 1;
	}
	log->info("Imported watchlist items from file");
	return 0;
}

std::string getCurrentDate(bool useLocalTime) {
	std::stringstream currentDateTime;
	// current date/time based on current system
	time_t ttNow = time(0);
	tm ptmNow;

	if (useLocalTime)
		localtime_s(&ptmNow, &ttNow);
	else
		gmtime_s(&ptmNow, &ttNow);

	currentDateTime << 1900 + ptmNow.tm_year << ".";

	//month
	if (ptmNow.tm_mon < 9)
		//Fill in the leading 0 if less than 10
		currentDateTime << "0" << 1 + ptmNow.tm_mon << ".";
	else
		currentDateTime << (1 + ptmNow.tm_mon) << ".";

	//day
	if (ptmNow.tm_mday < 10)
		currentDateTime << "0" << ptmNow.tm_mday;
	else
		currentDateTime << ptmNow.tm_mday;

	return currentDateTime.str();
}

bool finddate(const std::string &str) {
	std::string date = getCurrentDate(false);
	std::string buffer = str;
	reverse(buffer.begin(), buffer.end());
	buffer = buffer.substr(11, 10);
	reverse(buffer.begin(), buffer.end());
	return	!(date.find(buffer) != std::string::npos);
}