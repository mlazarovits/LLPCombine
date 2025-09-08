#include "JSONFactory.h"
#include "BuildFit.h"
#include <vector>
#include <string>
#include <filesystem> // Required for std::filesystem
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;
namespace fs = std::filesystem;


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
	

	JSONFactory* j = new JSONFactory(input_json);
//	BuildFit* BF = new BuildFit();
	
	std::vector<std::string> signals = j->GetSigProcs();
	//BF->BuildAsimovFit(j,"gogoG_2000_1000_500_10");

	//regenerate datacard directories
	fs::path dir_path = datacard_dir;
	fs::remove_all(dir_path);
	for( long unsigned int i=0; i<signals.size(); i++){
		/*std::vector<std::string> splitSignal = BFTool::SplitString(signals[i], "_");
		std::string mass = "";
      		for(long unsigned int i=1; i< splitSignal.size(); i++){
			mass += splitSignal[i];
		}*/
		BuildFit* BF = new BuildFit();
		std::filesystem::create_directories( datacard_dir+"/"+signals[i] );
		BF->BuildAsimovFit(j,signals[i], datacard_dir);
		//break;
	}
	cout << "Wrote datacards to " << datacard_dir << "/" << endl;
}
