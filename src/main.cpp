


#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"

int main() {
	double Lumi= 400.;
	SampleTool* ST = new SampleTool();
	
	//stringlist bkglist = {"Wjets", "Zjets", "Top", "Gjets","QCD"};
	stringlist bkglist = {"Wjets", "Zjets", "Gjets"};
	//stringlist siglist = {"gogoG"};
	stringlist siglist = {"gogoG","gogoZ","sqsqG"};
	string year = "18";
	stringlist datalist = {"DisplacedJet"+year};
	
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
	
	std::string pho1= "(nSelPhotons==1)";
	std::string pho2= "(nSelPhotons==2)";
	//std::string MMT = "&& (rjrASMass[1] > 2750) && (rjrAX2NQSum[1] > 0.275) && (Rv > 0.3)"; //old branch names
	//std::string LLL = "&& (rjrASMass[1] > 2000) && (rjrAX2NQSum[1] > 0.2) && (Rv > 0.0)"; 
	
	std::string MMT = "&& ( rjr_Mr[1] > 2750 ) && ( rjr_R[1] > 0.275 ) && ( rjr_Rv[1] > 0.3)";
	std::string LLL = "&& ( rjr_Mr[1] > 2000 ) && ( rjr_R[1] > 0.2 ) && (rjr_Rv[1] > 0.0)";
	std::string jets12 ="&&( ( (rjrNJetsJa[1] == 1) && (rjrNJetsJb[1] >= 1 ) ) || ( (rjrNJetsJa[1] >=1 ) && (rjrNJetsJb[1] == 1) ) )" ;
	std::string jets22="&& (rjrNJetsJa[1] >= 2) && (rjrNJetsJb[1] >= 2)";
	
	BFI->FilterRegions( "G1MMT11j", pho1+MMT+jets12);
	BFI->FilterRegions( "G1MMT22j", pho1+MMT+jets22);
	BFI->FilterRegions( "G2LLL", pho2+LLL);
	BFI->CreateBin("G1MMT11j");
	BFI->CreateBin("G1MMT22j");
	BFI->CreateBin("G2LLL");

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

	std::string outputJSON = "test_v36.json";	
	JSONFactory* json = new JSONFactory(BFI->analysisbins);
	json->WriteJSON("./json/"+outputJSON);
}
