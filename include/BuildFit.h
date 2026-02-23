#ifndef BUILDFIT_H
#define BUILDFIT_H

#include "JSONFactory.h"
#include "BuildFitTools.h"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <yaml-cpp/yaml.h>


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
typedef std::map<std::string, std::vector<std::string> > channelmap; 
using std::vector;
using std::string;
using std::map;
struct yamlSys{
        public:
                yamlSys(YAML::Node syst) :
                _init_val(-1){
                        _type = syst["type"].as<string>();
                        _init_val = syst["init_val"].as<double>();
                        _bins = syst["bins"].as<vector<string>>();

                };
                string _type;
                double _init_val;
                vector<string> _bins;
};


class BuildFit{
	public:
		//takes in fit config yaml file
		BuildFit(string infile);
		ch::CombineHarvester cb{};
	
		void PrepFit(JSONFactory* j, string signalPoint, vector<string> datakeys = {});
		void BuildShapeTransferFit(string signalPoint);

		ch::Categories BuildCats(JSONFactory* j);
		std::map<std::string, float> BuildAsimovData(JSONFactory* j);
        	std::vector<std::string> GetBkgProcs(JSONFactory* j);
		std::vector<std::string> GetDataProcs(JSONFactory* j);
		std::vector<std::string> ExtractSignalDetails( std::string signalPoint);
		std::vector<std::string> GetBinSet( JSONFactory* j);
		std::map<std::string, float> LoadObservations(JSONFactory* j);
		double GetStatFracError(JSONFactory* j, std::string binName, std::vector<std::string> bkgprocs );
		std::map<std::string, float> LoadDataProcesses(JSONFactory* j, std::vector<std::string> dataKeys);


		void BuildAsimovFit(JSONFactory* j, std::string signaPoint, std::string datacard_dir);
		void BuildABCDFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir, std::vector<std::string> ABCDbins);
		void BuildPseudoShapeTemplateFit(JSONFactory* j, JSONFactory* jup, JSONFactory* jdn, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void Build9binFitMC(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void Build9binFitData(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void BuildMultiChannel9bin(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);

		std::vector<std::string> sigkeys = { "gogoZ", "gogoG", "gogoGZ", "sqsqZ", "sqsqG", "sqsqGZ" };
		std::vector<std::string> datakeys = { "MET18", "DisplacedJet18"};


	private:
		channelmap _ch_ass; //channel association for shape transfer fit
		channelmap _bin_ass; //bin associations for each channel
		channelmap _abcd_bin_ass; //SR (key) to B, C, D (vals) for ABCD fit
		channelmap _abcd_ch_ass; //if channels are connected between ABCD fits
		vector<yamlSys> _systs; //extra systematics to connect channels, etc
		ch::Categories _cats;
		bool _asimov;
		std::map<std::string, float> _obs_rates;
		std::vector<std::string> _bkgprocs;
		std::vector<std::string> _signalDetails;
};
#endif
