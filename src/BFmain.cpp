#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <iostream>
#include <filesystem> // Required for std::filesystem
#include <cstdlib>    // Required for std::system

int main(){

	BuildFit BF("config/test_simple_fitconfig.yaml");
	string input_json = "json/testDelayedPhotonConfig.json";
	JSONFactory* j = new JSONFactory(input_json);
	BF.PrepFit(j, "gogoG_2000_1000_500_10");

	/*	
	std::vector<std::string> signals = j->GetSigProcs();
	//BF->BuildAsimovFit(j,"gogoG_2000_1000_500_10");

	std::vector<std::string> ABCDbins = {"G1CRA","G1CRB","G1CRC","G1CRD"};
	//channel map for combine paper fits
	//channelmap channelMap = {{"ch1",{ "bin1", "bin2", "bin3", "bin4", "bin5", "bin6", "bin7", "bin8", "bin9", "bin10"}} };
	
	//channel map for 9bin sv test fit
	//3channel fit
	channelmap channelMap = {
		{"ch1",{"Ch1CRHad00","Ch1CRHad10","Ch1CRHad20","Ch1CRHad01","Ch1CRHad11","Ch1CRHad21","Ch1CRHad02","Ch1CRHad12","Ch1CRHad22"}},
		{"ch2",{"Ch2CRHad00","Ch2CRHad10","Ch2CRHad20","Ch2CRHad01","Ch2CRHad11","Ch2CRHad21","Ch2CRHad02","Ch2CRHad12","Ch2CRHad22"}},
		{"ch3",{"Ch3CRLep00","Ch3CRLep10","Ch3CRLep20","Ch3CRLep01","Ch3CRLep11","Ch3CRLep21","Ch3CRLep02","Ch3CRLep12","Ch3CRLep22"}}
	};


	//regenerate datacard directories
	std::filesystem::path dir_path = datacard_dir;
	std::filesystem::remove_all(dir_path);
	for( long unsigned int i=0; i<1;i++){//signals.size(); i++){
		BuildFit* BF = new BuildFit();
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		//BF->BuildAsimovFit(j,signals[i], datacard_dir);
		//BF->BuildABCDFit( j, signals[i], datacard_dir, ABCDbins );
		//BF->BuildPseudoShapeTemplateFit(j,jUp,jDn, signals[i], datacard_dir, channelMap);
		//BF->Build9binFitMC(j,signals[i], datacard_dir, channelMap);
		//BF->Build9binFitData(j,signals[i], datacard_dir, channelMap);
		BF->BuildMultiChannel9bin(j,signals[i], datacard_dir, channelMap);
		//break;
	}
	*/	
}
