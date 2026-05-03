
#ifndef JSONFACT_H
#define JSONFACT_H
#include "nlohmann/json.hpp" // JSON lib
#include "BuildFitTools.h"
#include "ConfigParser.h"
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

class JSONFactory{

	public:
	JSONFactory(std::map<std::string, Bin*> analysisbins);
	JSONFactory(std::map<std::string, Bin*> analysisbins, const AnalysisConfig& c);
	JSONFactory(std::string filename);
	json j{};
	std::vector<std::string> GetSigProcs();
	void WriteJSON(std::string filename);

	std::vector<std::string> sigkeys = { "gogoZ", "gogoG", "gogoGZ", "sqsqZ", "sqsqG", "sqsqGZ" };

};

#endif
