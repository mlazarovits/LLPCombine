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
	
	//stringlist bkglist = {"Wjets", "Zjets", "Top", "Gjets","QCD"};
	stringlist bkglist = {"Wjets", "Zjets", "Gjets"};
	stringlist siglist = {"sqsqG"};//{"gogoG"};
	//stringlist siglist = {"gogoG","gogoZ","sqsqG"};
	
	ST->LoadBkgs( bkglist );
	ST->LoadSigs( siglist );
	
	ST->PrintDict(ST->BkgDict);
	ST->PrintDict(ST->SigDict);
	ST->PrintKeys(ST->SignalKeys);
	
	BuildFitInput* BFI = new BuildFitInput();
	BFI->LoadBkg_byMap(ST->BkgDict, Lumi);
	BFI->LoadSig_byMap(ST->SigDict, Lumi);
//	BFI->BuildRVBranch(); obselete
	

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

	//std::string H41s3000_Rs0p5 = "&& ( rjr_pHs41[1] > 3000 ) && ( rjr_Rs[1] > 0.5 ) && ( rjr_Rxa[1] != 1) && (rjr_Rxb[1] != 1)";
	//BFI->FilterRegions( "Ggt0_Hs41-3000_Rs-0p5", phogt0+H41s3000_Rs0p5);
	//BFI->CreateBin("Ggt0_Hs41-3000_Rs-0p5");

	std::string Mr1500_R0p6 = "&& ( rjr_Mr[1] > 1500 ) && ( rjr_R[1] > 0.6 )";
	BFI->FilterRegions( "Ggt0_Mr-1500_R-0p6", phogt0+Mr1500_R0p6);
	BFI->CreateBin("Ggt0_Mr-1500_R-0p6");

	//atlas cuts (stripped down)
	//https://arxiv.org/pdf/1712.02332
	//rjr variables are saved s.t. var[i] for i = 0 is phomet where the photon is treated as invisible and i = 1 is phojet where the photon is treated as visible
	/*
	std::string rjr_g1a = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.45) && ((rjr_pHts41[1] / rjr_pHs41[1]) >= 0.7) && (rjr_pHts41[1] > 1200) && (rjr_pHs11[1] > 700)";
	std::string rjr_g1b = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.45) && (rjr_pHts41[1] / rjr_pHs41[1] >= 0.7) && (rjr_pHts41[1] > 1400) && (rjr_pHs11[1] > 700)";
	
	std::string rjr_g2a = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.3) && (rjr_pHts41[1] / rjr_pHs41[1] >= 0.7) && (rjr_pHts41[1] > 1600) && (rjr_pHs11[1] > 800)";
	std::string rjr_g2b = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.3) && (rjr_pHts41[1] / rjr_pHs41[1] >= 0.7) && (rjr_pHts41[1] > 2000) && (rjr_pHs11[1] > 800)";
	
	std::string rjr_g3a = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.2) && (rjr_pHts41[1] / rjr_pHs41[1] >= 0.65) && (rjr_pHts41[1] > 2400) && (rjr_pHs11[1] > 900)";
	std::string rjr_g3b = "&& (rjr_pHs11[1] / rjr_pHs41[1] >= 0.2) && (rjr_pHts41[1] / rjr_pHs41[1] >= 0.65) && (rjr_pHts41[1] > 2800) && (rjr_pHs11[1] > 900)";
	
	std::string rjr_g4 = "&& (rjr_pHts41[1] / rjr_pHs41[1] >= 0.7) && (rjr_pHts41[1] > 3000) && (rjr_pHs11[1] > 1000)";
	BFI->FilterRegions("RJR-G1a",pho1+rjr_g1a);
	BFI->FilterRegions("RJR-G1b",pho1+rjr_g1b);
	BFI->FilterRegions("RJR-G2a",pho1+rjr_g2a);
	BFI->FilterRegions("RJR-G2b",pho1+rjr_g2b);
	BFI->FilterRegions("RJR-G3a",pho1+rjr_g3a);
	BFI->FilterRegions("RJR-G3b",pho1+rjr_g3b);
	BFI->FilterRegions("RJR-G4",pho1+rjr_g4);
	BFI->CreateBin("RJR-G1a");
	BFI->CreateBin("RJR-G1b");
	BFI->CreateBin("RJR-G2a");
	BFI->CreateBin("RJR-G2b");
	BFI->CreateBin("RJR-G3a");
	BFI->CreateBin("RJR-G3b");
	BFI->CreateBin("RJR-G4");
	*/


	//book operations
	countmap countResults = BFI->CountRegions(BFI->bkg_filtered_dataframes);
	countmap countResults_S = BFI->CountRegions(BFI->sig_filtered_dataframes); 
	
	summap sumResults = BFI->SumRegions("evtwt",BFI->bkg_filtered_dataframes );
	summap sumResults_S = BFI->SumRegions("evtwt",BFI->sig_filtered_dataframes);
	
	//initiate action
	BFI->ReportRegions(0);

	//compute errors and report bins
	errormap errorResults = BFI->ComputeStatError( countResults, BFI->bkg_evtwt );
	errormap errorResults_S = BFI->ComputeStatError( countResults_S, BFI->sig_evtwt);
	//BFI->FullReport( countResults, sumResults, errorResults );
	
	//aggregate maps into more easily useable classes
	BFI->ConstructBkgBinObjects( countResults, sumResults, errorResults );
	BFI->AddSigToBinObjects( countResults_S, sumResults_S, errorResults_S, BFI->analysisbins);
	BFI->PrintBins(1);

	JSONFactory* json = new JSONFactory(BFI->analysisbins);
	cout << "Writing output json to json/" << outputJSON << endl;
	json->WriteJSON("./json/"+outputJSON);
}
