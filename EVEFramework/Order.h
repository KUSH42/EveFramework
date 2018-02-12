#pragma once

#include <string>
#include <ctime>
#include <vector>

struct Order {
	double price;
	double volRemaining;
	std::string typeID;
	int range;
	std::string orderID;
	double volEntered;
	double minVolume;
	bool bid;
	std::tm issueDate;
	int duration;
	std::string stationID;
	std::string regionID;
	std::string solarSystemID;
	int jumps;

	Order(const std::vector<std::string> &v_order);
};