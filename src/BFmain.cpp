
#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <iostream>
#include <filesystem> // Required for std::filesystem
#include <cstdlib>    // Required for std::system

int main(){

	
	//std::string datacard_dir = "datacards_abcd";
	//std::string input_json = "./json/test_v37_data.json";
	//std::string datacard_dir = "datacards_pseudoshape2bbb";
	//std::string datacard_dir = "datacards_pseudoshape3automc";
	//std::string datacard_dir = "datacards_pseudoshape4automconly";
       // std::string datacard_dir = "datacards_pseudoshape4mclnN";
//	std::string datacard_dir = "datacards_pseudoshape5mclnNupdn";
	//std::string datacard_dir = "datacards_9binMC";
	std::string datacard_dir = "datacards_9binData_bficonfig";

//	std::string jsonShapeUp ="./json/shapeUpTest.json";
//	std::string jsonShapeDn ="./json/shapeDnTest.json";
//	std::string jsonNominal ="./json/shapeNominalTest.json";

	//std::string input_json = "./json/test_9binCR_SV_wMC.json";
	//std::string input_json = "./json/test_9binCR_SV_noMC.json";
	std::string input_json= "./json/test9binConfig.json";

	// Load JSON and get signal processes
	JSONFactory* j = new JSONFactory(input_json);
//	JSONFactory*  j = new JSONFactory(jsonNominal);
//	JSONFactory* jUp = new JSONFactory(jsonShapeUp);
//	JSONFactory* jDn = new JSONFactory(jsonShapeDn);
//	BuildFit* BF = new BuildFit();
	
	std::vector<std::string> signals = j->GetSigProcs();
	//BF->BuildAsimovFit(j,"gogoG_2000_1000_500_10");

	std::vector<std::string> ABCDbins = {"G1CRA","G1CRB","G1CRC","G1CRD"};
	//channel map for combine paper fits
	//channelmap channelMap = {{"ch1",{ "bin1", "bin2", "bin3", "bin4", "bin5", "bin6", "bin7", "bin8", "bin9", "bin10"}} };
	
	//channel map for 9bin sv test fit
	channelmap channelMap = {
		{"chHad1",{"CRHad00","CRHad10","CRHad20","CRHad01","CRHad11","CRHad21","CRHad02","CRHad12","CRHad22"}},
		{"chLep1",{"CRLep00","CRLep10","CRLep20","CRLep01","CRLep11","CRLep21","CRLep02","CRLep12","CRLep22"}}
	};

	//regenerate datacard directories
	std::filesystem::path dir_path = datacard_dir;
	std::filesystem::remove_all(dir_path);
	for( long unsigned int i=0; i<signals.size(); i++){
		BuildFit* BF = new BuildFit();
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		//BF->BuildAsimovFit(j,signals[i], datacard_dir);
		//BF->BuildABCDFit( j, signals[i], datacard_dir, ABCDbins );
		//BF->BuildPseudoShapeTemplateFit(j,jUp,jDn, signals[i], datacard_dir, channelMap);
		//BF->Build9binFitMC(j,signals[i], datacard_dir, channelMap);
		BF->Build9binFitData(j,signals[i], datacard_dir, channelMap);
		break;
	}
	
}
