#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"

int main(int argc, char *argv[]){
	bool hprint = false;
	std::string outputJSON = "test_v36.json";	
        for(int i = 0; i < argc; i++){
                if(strncmp(argv[i],"--help", 6) == 0){
                        hprint = true;
                }
                if(strncmp(argv[i],"-h", 2) == 0){
                        hprint = true;
                }
                if(strncmp(argv[i],"--output", 8) == 0){
                	i++;
                        outputJSON = string(argv[i])+".json";
		}
                if(strncmp(argv[i],"-o", 2) == 0){
                	i++;
                        outputJSON = string(argv[i])+".json";
                }

	}
	if(hprint){
                cout << "Making BFI jsons for BuildFit" << endl;
                cout << "Usage: " << argv[0] << " [options]" << endl;
                cout << "  options:" << endl;
                cout << "   --help(-h)                    print options" << endl;
                cout << "   --output(-o) [oname]          set output json name" << endl;

		return -1;
	}
	double Lumi= 400.;
	SampleTool* ST = new SampleTool();
	
	stringlist bkglist = {"Wjets", "Zjets", "Top", "Gjets","QCD", "Box"};
	//stringlist bkglist = {"Wjets", "Zjets", "Gjets"};
	stringlist siglist = {"sqsqG","gogoG"};
	string year = "18";
	stringlist datalist = {};//{"DisplacedJet"+year};
	//stringlist siglist = {"gogoG","gogoZ","sqsqG"};
	ST->LoadBkgs( bkglist );
	ST->LoadSigs( siglist );
	ST->LoadData( datalist );

	ST->PrintDict(ST->BkgDict);
	ST->PrintDict(ST->DataDict);
	ST->PrintDict(ST->SigDict);
	ST->PrintKeys(ST->SignalKeys);
	
	BuildFitInput* BFI = new BuildFitInput();
	BFI->LoadData_byMap(ST->DataDict, Lumi);
//	BFI->BuildRVBranch(); obselete
	BFI->LoadBkg_byMap(ST->BkgDict, Lumi);
	BFI->LoadSig_byMap(ST->SigDict, Lumi);
	
	std::string phogt0= "(nSelPhotons>0)";
	std::string pho1= "(nSelPhotons==1)";
	std::string pho2= "(nSelPhotons==2)";
	//std::string MMT = "&& (rjrASMass[1] > 2750) && (rjrAX2NQSum[1] > 0.275) && (Rv > 0.3)"; //old branch names
	//std::string LLL = "&& (rjrASMass[1] > 2000) && (rjrAX2NQSum[1] > 0.2) && (Rv > 0.0)"; 
	
	std::string MMT = "&& ( rjr_Mr[1] > 2750 ) && ( rjr_R[1] > 0.275 ) && ( rjr_Rv[1] > 0.3)";
	std::string LLL = "&& ( rjr_Mr[1] > 2000 ) && ( rjr_R[1] > 0.2 ) && (rjr_Rv[1] > 0.0)";
	std::string jets12 ="&&( ( (rjrNJetsJa[1] == 1) && (rjrNJetsJb[1] >= 1 ) ) || ( (rjrNJetsJa[1] >=1 ) && (rjrNJetsJb[1] == 1) ) )" ;
	std::string jets22="&& (rjrNJetsJa[1] >= 2) && (rjrNJetsJb[1] >= 2)";
	//BFI->FilterRegions( "G1MMT11j", pho1+MMT+jets12);
	//BFI->FilterRegions( "G1MMT22j", pho1+MMT+jets22);
	//BFI->FilterRegions( "G1LLL", pho1+LLL);
	//BFI->FilterRegions( "Ggt0LLL", phogt0+LLL);
	//BFI->FilterRegions( "G1MMT", pho1+MMT);
	////
	////BFI->CreateBin("G1MMT11j");
	////BFI->CreateBin("G1MMT22j");
	//BFI->CreateBin("G1MMT");
	//BFI->CreateBin("Ggt0LLL");

	std::string H41s1600_Rs0p5 = "&& ( rjr_pHs41[1] > 1600 ) && ( rjr_Rs[1] > 0.5 ) && ( rjr_Rxa[1] != 1) && (rjr_Rxb[1] != 1)";
	BFI->FilterRegions( "Ggt0_Hs41-1600_Rs-0p5", phogt0+H41s1600_Rs0p5);
	BFI->CreateBin("Ggt0_Hs41-1600_Rs-0p5");

	//std::string Mr1700_R0p6 = "&& ( rjr_Mr[1] > 1700 ) && ( rjr_R[1] > 0.6 )";
	//BFI->FilterRegions( "Ggt0_Mr-1700_R-0p6", phogt0+Mr1700_R0p6);
	//BFI->CreateBin("Ggt0_Mr-1700_R-0p6");



	//book operations
	countmap countResults = BFI->CountRegions(BFI->bkg_filtered_dataframes);
	countmap countResults_S = BFI->CountRegions(BFI->sig_filtered_dataframes); 
	//only do raw event counts for data
	countmap countResults_obs = BFI->CountRegions(BFI->data_filtered_dataframes);

	summap sumResults = BFI->SumRegions("evtwt",BFI->bkg_filtered_dataframes );
	summap sumResults_S = BFI->SumRegions("evtwt",BFI->sig_filtered_dataframes);
	summap sumResults_obs = BFI->SumRegions("evtwt",BFI->data_filtered_dataframes);
	
	//initiate action
	BFI->ReportRegions(0);

	//compute errors and report bins
	errormap errorResults = BFI->ComputeStatError( countResults, BFI->bkg_evtwt );
	errormap errorResults_S = BFI->ComputeStatError( countResults_S, BFI->sig_evtwt);
	//TODO - won't use weighted events in fit for data
	errormap errorResults_obs = BFI->ComputeStatError( countResults_obs, BFI->data_evtwt);
	
	//BFI->FullReport( countResults, sumResults, errorResults );
	
	//aggregate maps into more easily useable classes
	BFI->ConstructBkgBinObjects( countResults, sumResults, errorResults );
	BFI->AddSigToBinObjects( countResults_S, sumResults_S, errorResults_S, BFI->analysisbins);
	//only write data to json if data samples are specified
	if(datalist.size() > 0) BFI->AddDataToBinObjects( countResults_obs, sumResults_obs, errorResults_obs, BFI->analysisbins);
	BFI->PrintBins(1);

	JSONFactory* json = new JSONFactory(BFI->analysisbins);
	cout << "Writing output json to json/" << outputJSON << endl;
	json->WriteJSON("./json/"+outputJSON);
}
