#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <iostream>
#include <filesystem> // Required for std::filesystem
#include <cstdlib>    // Required for std::system
#include <exception>
#include "ConfigParser.h"
#include "ArgumentParser.h"
using std::cout;
using std::endl;
int main(int argc, char* argv[]){
	string inconfig;
	string input_json;
	string extra_name;
	bool hprint = false;
	bool crfit = false;
	for(int i = 0; i < argc; i++){
		if(strncmp(argv[i],"--help", 6) == 0){
                	hprint = true;
                }
                if(strncmp(argv[i],"-h", 2) == 0){
                        hprint = true;
                }
		if(strncmp(argv[i],"--config", 8) == 0){
                	i++;
                	inconfig = string(argv[i]);
                }
		if(strncmp(argv[i],"-c", 2) == 0){
                	i++;
                	inconfig = string(argv[i]);
                }
		if(strncmp(argv[i],"--BFI", 5) == 0){
                	i++;
                	input_json = string(argv[i]);
                }
		if(strncmp(argv[i],"-i", 2) == 0){
                	i++;
                	input_json = string(argv[i]);
                }
		if(strncmp(argv[i],"--CRFit", 7) == 0){
			crfit = true;
		}
		if(strncmp(argv[i],"-e", 2) == 0){
                	i++;
                	extra_name = string(argv[i]);
                }
		if(strncmp(argv[i],"--extra", 7) == 0){
                	i++;
                	extra_name = string(argv[i]);
                }

	}
	if(hprint){
		cout << "Usage: " << argv[0] << " [options]" << endl;
                cout << "  options:" << endl;
                cout << "   --help(-h)                           print options" << endl;
                cout << "   --config(-c) [file]                  fit config" << endl;
                cout << "   --BFI(-i) [file]                     build fit input (json file)" << endl;
                cout << "   --CRFit                              run single signal for CR fit" << endl;
                cout << "   --extra(-e)                          additional name for datacard dir" << endl;
		return 0;
	}	

	if(input_json == ""){
		cout << "Please specify an input BFI json file with --BFI(-i) [json]." << endl;
		return -1;
	}
	if(inconfig == ""){
		cout << "Please specify a fit configuration yaml file with --config(-c) [yaml]." << endl;
		return -1;
	}

	try{
		//TODO - automate fit config writing
		JSONFactory* j = new JSONFactory(input_json);
		std::vector<std::string> signals = j->GetSigProcs();

		///don't run over this signal if its not in the BFI
		//if(find(signals.begin(), signals.end(), signal) == signals.end())
		//	return -1;

		std::string datacard_dir = "datacards";
		long unsigned int nsig = signals.size();
		if(crfit)
			nsig = 1;
		for( long unsigned int i=0; i<nsig;i++){
			BuildFit BF(inconfig);
			string fitname = BF.GetFitName();
			std::cout << "fitname " << fitname << std::endl;
			if(fitname.size() > 1)
				datacard_dir += "_"+fitname;
			if(extra_name.size() > 1)
				datacard_dir += "_"+extra_name;
			datacard_dir += "/"+signals[i];
			cout << "Datacard dir " << datacard_dir << endl;
			std::filesystem::path dir_path = datacard_dir;
			//recreate datacards
			std::filesystem::remove_all(dir_path);
			std::filesystem::create_directories( datacard_dir );
			cout << "sig " << signals[i] << endl;
			//one BF instances per signal point - creates one CH object and therefore one datacard per signal point
			BF.PrepFit(j, signals[i]);
			//do fit - function won't do anything if their corresponding section in the config yaml isn't filled
			BF.BuildShapeTransferFit();
			BF.BuildABCDFit();
			BF.SetObservations(); 
			BF.DoSystematics();
			//write datacard
			BF.WriteDatacard(datacard_dir, true);
			datacard_dir = "datacards";

			//needs to come after fit initialization because asimov observations are set differently
			//depending on which fit is run (for now)
			//BF.BuildABCDFitChannelToChannel();
		}
	}
	catch(const std::exception& e){
		std::cerr << "\nBF.x failed while building datacards" << std::endl;
		std::cerr << "  input JSON: " << input_json << std::endl;
		std::cerr << "  fit config: " << inconfig << std::endl;
		std::cerr << "  error: " << e.what() << std::endl;
		return 2;
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
