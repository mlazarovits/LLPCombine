#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <iostream>
#include <filesystem> // Required for std::filesystem
#include <cstdlib>    // Required for std::system

int main(){

	string input_json = "json/testSimpleBFI.json";
	JSONFactory* j = new JSONFactory(input_json);
	std::vector<std::string> signals = j->GetSigProcs();
	std::string datacard_dir = "datacards_test/";

	//make datacards for each signal point
	//regenerate datacard directories
	std::filesystem::path dir_path = datacard_dir;
	std::filesystem::remove_all(dir_path);
	for( long unsigned int i=0; i<1;i++){//signals.size(); i++){
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		//one BF instances per signal point - creates one CH object and therefore one datacard per signal point
		BuildFit BF("config/test_simple_fitconfig.yaml");
		BF.PrepFit(j, signals[i]);
		//do fit
		//BF.BuildShapeTransferFit(signals[i]);
		BF.BuildABCDFit(signals[i]);
		BF.DoSystematics();
		//write datacard
		BF.WriteDatacard(datacard_dir, signals[i], true);
	}

	/*
	std::vector<std::string> ABCDbins = {"G1CRA","G1CRB","G1CRC","G1CRD"};
	//channel map for test fit
	channelmap channelMap = {
		{"ch1",{"Ch1Pho1CR00","Ch1Pho1CR10","Ch1Pho1CR01","Ch1Pho1CR11"}},
		{"ch2",{"Ch2Pho2CR00","Ch2Pho2CR10","Ch2Pho2CR01","Ch2Pho2CR11"}},
	};
	//regenerate datacard directories
	datacard_dir = "datacards/";
	std::filesystem::path dir_path = datacard_dir;
	std::filesystem::remove_all(dir_path);
	for( long unsigned int i=0; i<1;i++){//signals.size(); i++){
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		BuildFit* BF = new BuildFit();
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
