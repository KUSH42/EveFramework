#pragma once
#include <vector>
#include <string>
#include "..\Order.h"

struct LogUtil {
	std::shared_ptr<spdlog::logger> log;

	std::vector<Order> importAnalysis(const std::string & searchString);
	LogUtil();
	int marketImport(const std::string &filepath, std::vector<std::vector<std::string>> &p_v_strings);
	int watchlistImport(const std::string &filepath, std::vector<std::string> &p_v_strings);
};
