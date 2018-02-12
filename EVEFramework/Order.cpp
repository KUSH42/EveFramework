#include "Order.h"
const std::string TRUE = "True";

Order::Order(const std::vector<std::string> &v_order) {
	this->price = atoi(v_order[0].c_str());
	this->volRemaining = atoi(v_order[1].c_str());
	this->typeID = v_order[2];
	this->range = atoi(v_order[3].c_str());
	this->orderID = v_order[4];
	this->volEntered = atoi(v_order[5].c_str());
	this->minVolume = atof(v_order[6].c_str());
	if (v_order[7] == TRUE) {
		this->bid = true;
	}
	else {
		this->bid = false;
	}
	int day, month, year, minute, hour, second;
	sscanf_s(v_order[8].c_str(), "%d-%d-%d %d:%d:%d.", &year, &month, &day, &hour, &minute, &second);
	this->issueDate.tm_year = year-1900;
	this->issueDate.tm_mon = month - 1;
	this->issueDate.tm_mday = day;
	this->issueDate.tm_hour = hour;
	this->issueDate.tm_min = minute;
	this->issueDate.tm_sec = second;
	this->duration = atoi(v_order[9].c_str());
	this->stationID = v_order[10];
	this->regionID = v_order[11];
	this->solarSystemID = v_order[12];
	this->jumps = atoi(v_order[13].c_str());;
}