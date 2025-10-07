
#ifndef BUILDFIT_H
#define BUILDFIT_H

#include "JSONFactory.h"
#include "BuildFitTools.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
//using ch::syst::SystMapFunc;
using ch::syst::SystMap;
using ch::syst::SystMapFunc;
using ch::syst::bin;
using json = nlohmann::json;

class BuildFit{
	
	public:
	ch::CombineHarvester cb{};
	
//	void BuildAsimovFit(JSONFactory* j);

	ch::Categories BuildCats(JSONFactory* j);
	std::map<std::string, float> BuildAsimovData(JSONFactory* j);
        std::vector<std::string> GetBkgProcs(JSONFactory* j);
	std::vector<std::string> ExtractSignalDetails( std::string signalPoint);
	std::vector<std::string> GetBinSet( JSONFactory* j);
       
	void BuildAsimovFit(JSONFactory* j, std::string signaPoint, std::string datacard_dir);
	
	
	std::vector<std::string> sigkeys = { "gogoZ", "gogoG", "gogoGZ", "sqsqZ", "sqsqG", "sqsqGZ" };
	

};
#endif
