#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <iostream>
#include <filesystem> // Required for std::filesystem
<<<<<<< HEAD
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;
namespace fs = std::filesystem;

=======
#include <cstdlib>    // Required for std::system
>>>>>>> d51a4eda2117ff871089f7d3066a3f681930c0fa

int main(int argc, char *argv[]){
        bool hprint = false;
	std::string datacard_dir = "datacards_eos";
	std::string input_json = "./json/test_eos.json";
        for(int i = 0; i < argc; i++){
                if(strncmp(argv[i],"--help", 6) == 0){
                        hprint = true;
                }
                if(strncmp(argv[i],"-h", 2) == 0){
                        hprint = true;
                }
                if(strncmp(argv[i],"--input", 7) == 0){
                        i++;
                        input_json = string(argv[i]);
                }
                if(strncmp(argv[i],"-i", 2) == 0){
                        i++;
                        input_json = string(argv[i]);
                }
                if(strncmp(argv[i],"--output", 8) == 0){
                        i++;
                        datacard_dir = string(argv[i]);
                }
                if(strncmp(argv[i],"-o", 2) == 0){
                        i++;
                        datacard_dir = string(argv[i]);
                }

<<<<<<< HEAD
        }
        if(hprint){
                cout << "Making BFI jsons for BuildFit" << endl;
                cout << "Usage: " << argv[0] << " [options]" << endl;
                cout << "  options:" << endl;
                cout << "   --help(-h)                    print options" << endl;
                cout << "   --input(-i) [inname]          set input json" << endl;
                cout << "   --output(-o) [oname]          set output dir name" << endl;
                return -1;
        }
//	std::string datacard_dir = "datacards";
//	std::string input_json = "test.json";
	//std::string datacard_dir = "datacards_22j";
	//std::string input_json = "test_G1MMT22j.json";
	//std::string datacard_dir = "datacards_11j";
        //std::string input_json = "test_G1MMT11j.json";
	//std::string datacard_dir = "datacards_2GLLL";
        //std::string input_json = "test_G2LLL.json";


	string name = input_json.substr(input_json.find("json/"),input_json.find(".json"));
	name = name.substr(name.find("/")+1);
	datacard_dir = "datacards_"+name;
	cout << "datacard dir " << datacard_dir << endl;
=======
	
	//std::string datacard_dir = "datacards_abcd";
	//std::string input_json = "./json/test_v37_data.json";
	//std::string datacard_dir = "datacards_pseudoshape2bbb";
	//std::string datacard_dir = "datacards_pseudoshape3automc";
	//std::string datacard_dir = "datacards_pseudoshape4automconly";
       // std::string datacard_dir = "datacards_pseudoshape4mclnN";
//	std::string datacard_dir = "datacards_pseudoshape5mclnNupdn";
	//std::string datacard_dir = "datacards_9binMC";
	//std::string datacard_dir = "datacards_9binData_bficonfig";
	//std::string datacard_dir = "datacards_9bin3ch";
//	std::string datacard_dir = "datacards_3ch_cleaned_v43";
//	std::string datacard_dir = "datacards_2photon_prompt";
	std::string datacard_dir = "datacards_2photon_prompt4bin";
//	std::string jsonShapeUp ="./json/shapeUpTest.json";
//	std::string jsonShapeDn ="./json/shapeDnTest.json";
//	std::string jsonNominal ="./json/shapeNominalTest.json";

	//std::string input_json = "./json/test_9binCR_SV_wMC.json";
	//std::string input_json = "./json/test_9binCR_SV_noMC.json";
	//std::string input_json= "./json/test9binConfig.json";
	//std::string input_json= "./json/SV3chCR_MET18_v42.json";
	//std::string input_json= "./json/SV3chCR_MET18_v43.json";
	//std::string input_json= "./json/photon_prompt_simple.json";
	std::string input_json= "./json/photon_prompt_simple4bin.json";
>>>>>>> d51a4eda2117ff871089f7d3066a3f681930c0fa

	// Load JSON and get signal processes
	JSONFactory* j = new JSONFactory(input_json);
//	JSONFactory*  j = new JSONFactory(jsonNominal);
//	JSONFactory* jUp = new JSONFactory(jsonShapeUp);
//	JSONFactory* jDn = new JSONFactory(jsonShapeDn);
//	BuildFit* BF = new BuildFit();
	
	std::vector<std::string> signals = j->GetSigProcs();
	//BF->BuildAsimovFit(j,"gogoG_2000_1000_500_10");

<<<<<<< HEAD
=======
	std::vector<std::string> ABCDbins = {"G1CRA","G1CRB","G1CRC","G1CRD"};
	//channel map for combine paper fits
	//channelmap channelMap = {{"ch1",{ "bin1", "bin2", "bin3", "bin4", "bin5", "bin6", "bin7", "bin8", "bin9", "bin10"}} };
	
	//channel map for 9bin sv test fit
	/*
	channelmap channelMap = {
		{"chHad1",{"CRHad00","CRHad10","CRHad20","CRHad01","CRHad11","CRHad21","CRHad02","CRHad12","CRHad22"}},
		{"chLep1",{"CRLep00","CRLep10","CRLep20","CRLep01","CRLep11","CRLep21","CRLep02","CRLep12","CRLep22"}}
	};
	*/
	//3channel fit
	channelmap channelMap = {
		{"ch1",{"Ch1CRHad00","Ch1CRHad10","Ch1CRHad20","Ch1CRHad01","Ch1CRHad11","Ch1CRHad21","Ch1CRHad02","Ch1CRHad12","Ch1CRHad22"}},
		{"ch2",{"Ch2CRHad00","Ch2CRHad10","Ch2CRHad20","Ch2CRHad01","Ch2CRHad11","Ch2CRHad21","Ch2CRHad02","Ch2CRHad12","Ch2CRHad22"}},
		{"ch3",{"Ch3CRLep00","Ch3CRLep10","Ch3CRLep20","Ch3CRLep01","Ch3CRLep11","Ch3CRLep21","Ch3CRLep02","Ch3CRLep12","Ch3CRLep22"}}
	};


>>>>>>> d51a4eda2117ff871089f7d3066a3f681930c0fa
	//regenerate datacard directories
	std::filesystem::path dir_path = datacard_dir;
	std::filesystem::remove_all(dir_path);
	for( long unsigned int i=0; i<signals.size(); i++){
		BuildFit* BF = new BuildFit();
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		BF->BuildAsimovFit(j,signals[i], datacard_dir);
<<<<<<< HEAD
		//break;
	}
	cout << "Wrote datacards to " << datacard_dir << "/" << endl;
=======
		//BF->BuildABCDFit( j, signals[i], datacard_dir, ABCDbins );
		//BF->BuildPseudoShapeTemplateFit(j,jUp,jDn, signals[i], datacard_dir, channelMap);
		//BF->Build9binFitMC(j,signals[i], datacard_dir, channelMap);
		//BF->Build9binFitData(j,signals[i], datacard_dir, channelMap);
		//BF->BuildMultiChannel9bin(j,signals[i], datacard_dir, channelMap);
		//break;
	}
	
>>>>>>> d51a4eda2117ff871089f7d3066a3f681930c0fa
}
