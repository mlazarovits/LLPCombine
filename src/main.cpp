


#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"

int main() {
	//double Lumi= 10.;
	double Lumi= 400.;
	SampleTool* ST = new SampleTool();
	
	stringlist bkglist = {"Wjets", "Zjets", "Top", "Gjets","QCD", "Box"};
	stringlist siglist = {"gogoG"};//{"gogoG","gogoZ","sqsqG"};
	stringlist datalist = {};//{"DisplacedJet17"};//{"DisplacedJet17","DisplacedJet18"};
	
	ST->LoadBkgs( bkglist );
	ST->LoadSigs( siglist );
	ST->LoadData( datalist );

	ST->PrintDict(ST->BkgDict);
	ST->PrintDict(ST->DataDict);
	ST->PrintDict(ST->SigDict);
	ST->PrintKeys(ST->SignalKeys);
	
	BuildFitInput* BFI = new BuildFitInput();
	BFI->LoadData_byMap(ST->DataDict);
	BFI->LoadBkg_byMap(ST->BkgDict, Lumi);
	BFI->LoadSig_byMap(ST->SigDict, Lumi);

	std::string pho0= "(nSelPhotons==0)";	
	std::string pho1= "(nSelPhotons==1)";
	std::string pho2= "(nSelPhotons==2)";
		
	std::string MMT = "&& ( rjr_Mr[1] > 2750 ) && ( rjr_R[1] > 0.275 ) && ( rjr_Rv[1] > 0.3)";
	std::string LLL = "&& ( rjr_Mr[1] > 2000 ) && ( rjr_R[1] > 0.2 ) && (rjr_Rv[1] > 0.0)";
	std::string jets12 ="&&( ( (rjrNJetsJa[1] == 1) && (rjrNJetsJb[1] >= 1 ) ) || ( (rjrNJetsJa[1] >=1 ) && (rjrNJetsJb[1] == 1) ) )" ;
	std::string jets22="&& (rjrNJetsJa[1] >= 2) && (rjrNJetsJb[1] >= 2)";
        
	std::string CR_A = "&& ( rjr_Ms[1] < 2000 ) && ( rjr_Ms[1] > 1500 ) && ( rjr_Rs[1] < 0.1 )";
	std::string CR_B = "&& ( rjr_Ms[1] < 2000 ) && ( rjr_Ms[1] > 1500 ) && ( rjr_Rs[1] > 0.1 ) && ( rjr_Rs[1] < 0.2 )";
	std::string CR_C = "&& ( rjr_Ms[1] < 1500 ) && ( rjr_Ms[1] > 1000  ) && ( rjr_Rs[1] < 0.1 )";
	std::string CR_D = "&& ( rjr_Ms[1] < 1500 ) && ( rjr_Ms[1] > 1000 ) && ( rjr_Rs[1] > 0.1 ) && (rjr_Rs[1] < 0.2 )";
	std::string softPhoton= "&& (selPhoPt[0] < 50)";

		
	BFI->FilterRegions( "G1MMT11j", pho1+MMT+jets12);
	BFI->FilterRegions( "G1MMT22j", pho1+MMT+jets22);
	BFI->FilterRegions( "G2LLL", pho2+LLL);
	BFI->CreateBin("G1MMT11j");
	BFI->CreateBin("G1MMT22j");
	BFI->CreateBin("G2LLL");
	

	//CR ABCD test regions
	/*	
	BFI->FilterRegions( "G1CRA", pho1+CR_A+softPhoton);
        BFI->FilterRegions( "G1CRB", pho1+CR_B+softPhoton);
        BFI->FilterRegions( "G1CRC", pho1+CR_C+softPhoton);
	BFI->FilterRegions( "G1CRD", pho1+CR_D+softPhoton);
        BFI->CreateBin("G1CRA");
        BFI->CreateBin("G1CRB");
        BFI->CreateBin("G1CRC");
	BFI->CreateBin("G1CRD");
	*/


	//book operations
	countmap countResults = BFI->CountRegions(BFI->bkg_filtered_dataframes);
	countmap countResults_S = BFI->CountRegions(BFI->sig_filtered_dataframes); 
	countmap countResults_obs = BFI->CountRegions(BFI->data_filtered_dataframes);

	summap sumResults = BFI->SumRegions("evtwt",BFI->bkg_filtered_dataframes );
	summap sumResults_S = BFI->SumRegions("evtwt",BFI->sig_filtered_dataframes);
	summap sumResults_obs = BFI->SumRegions("evtwt",BFI->data_filtered_dataframes);
	
	//initiate action
	BFI->ReportRegions(0);

	//compute errors and report bins
	errormap errorResults = BFI->ComputeStatError( countResults, BFI->bkg_evtwt );
	errormap errorResults_S = BFI->ComputeStatError( countResults_S, BFI->sig_evtwt);
	errormap errorResults_obs = BFI->ComputeStatError( countResults_obs, BFI->data_evtwt);
	
	//BFI->FullReport( countResults, sumResults, errorResults );
	
	//aggregate maps into more easily useable classes
	BFI->ConstructBkgBinObjects( countResults, sumResults, errorResults );
	BFI->AddSigToBinObjects( countResults_S, sumResults_S, errorResults_S, BFI->analysisbins);
	BFI->AddDataToBinObjects( countResults_obs, sumResults_obs, errorResults_obs, BFI->analysisbins);
	BFI->PrintBins(1);

	std::string outputJSON = "test_v38.json";	
	JSONFactory* json = new JSONFactory(BFI->analysisbins);
	json->WriteJSON("./json/"+outputJSON);
}
