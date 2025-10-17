


#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"

int main() {
	double Lumi=4.;
	//double Lumi= 400.;
	SampleTool* ST = new SampleTool();
	
	stringlist bkglist = {};//{"Wjets", "Zjets", "Top", "Gjets","QCD", "Box"};
	stringlist siglist = {"gogoZ"};//{"gogoG","gogoZ","sqsqG"};
	stringlist datalist = {"MET18"};//{"DisplacedJet17"};//{"DisplacedJet17","DisplacedJet18"};
	
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

	//sv shape fit testing
	std::string nHad1 = "(SV_nHadronic == 1) && ";
        std::string nLep1 = "(SV_nLeptonic == 1) && ";
	std::string CRHadDxySig = "( HadronicSV_dxySig[0] > 100) && (HadronicSV_dxySig[0] < 1000) && ";
	std::string CRLepDxySig = "( LeptonicSV_dxySig[0] > 100) && (LeptonicSV_dxySig[0] < 1000) && ";

	std::string CRMET = "(selMet > 100) && (selMet < 150 ) && ";
	//photon orthongoal
	std::string pho0= "(nSelPhotons==0) && ";

	std::string bin00 = "(rjr_Ms[1] >= 1000) && (rjr_Ms[1] <2000) && (rjr_Rs[1] >= 0.15) && (rjr_Rs[1] < 0.3)";
	std::string bin10 = "(rjr_Ms[1] >= 2000) && (rjr_Ms[1] <3000) && (rjr_Rs[1] >= 0.15) && (rjr_Rs[1] < 0.3)";
	std::string bin20 = "(rjr_Ms[1] >= 3000) && (rjr_Rs[1] >= 0.15) && (rjr_Rs[1] < 0.3)";
	
	std::string bin01 = "(rjr_Ms[1] >= 1000) && (rjr_Ms[1] <2000) && (rjr_Rs[1] >= 0.3) && (rjr_Rs[1] < 0.4)";
        std::string bin11 = "(rjr_Ms[1] >= 2000) && (rjr_Ms[1] <3000) && (rjr_Rs[1] >= 0.3) && (rjr_Rs[1] < 0.4)";
        std::string bin21 = "(rjr_Ms[1] >= 3000) && (rjr_Rs[1] >= 0.3) && (rjr_Rs[1] < 0.4)";

	std::string bin02 = "(rjr_Ms[1] >= 1000) && (rjr_Ms[1] <2000) && (rjr_Rs[1] > 0.4)";
        std::string bin12 = "(rjr_Ms[1] >= 2000) && (rjr_Ms[1] <3000) && (rjr_Rs[1] > 0.4)";
        std::string bin22 = "(rjr_Ms[1] >= 3000) && (rjr_Rs[1] > 0.4)";


              //CR ABCD test regions
        BFI->FilterRegions( "CRHad00", nHad1+CRHadDxySig+CRMET+pho0+bin00);
        BFI->FilterRegions( "CRHad10", nHad1+CRHadDxySig+CRMET+pho0+bin10);
        BFI->FilterRegions( "CRHad20", nHad1+CRHadDxySig+CRMET+pho0+bin20);
        BFI->FilterRegions( "CRHad01", nHad1+CRHadDxySig+CRMET+pho0+bin01);
        BFI->FilterRegions( "CRHad11", nHad1+CRHadDxySig+CRMET+pho0+bin11);
        BFI->FilterRegions( "CRHad21", nHad1+CRHadDxySig+CRMET+pho0+bin21);
	BFI->FilterRegions( "CRHad02", nHad1+CRHadDxySig+CRMET+pho0+bin02);
        BFI->FilterRegions( "CRHad12", nHad1+CRHadDxySig+CRMET+pho0+bin12);
        BFI->FilterRegions( "CRHad22", nHad1+CRHadDxySig+CRMET+pho0+bin22);

	BFI->FilterRegions( "CRLep00", nLep1+CRLepDxySig+CRMET+pho0+bin00);
        BFI->FilterRegions( "CRLep10", nLep1+CRLepDxySig+CRMET+pho0+bin10);
        BFI->FilterRegions( "CRLep20", nLep1+CRLepDxySig+CRMET+pho0+bin20);
        BFI->FilterRegions( "CRLep01", nLep1+CRLepDxySig+CRMET+pho0+bin01);
        BFI->FilterRegions( "CRLep11", nLep1+CRLepDxySig+CRMET+pho0+bin11);
        BFI->FilterRegions( "CRLep21", nLep1+CRLepDxySig+CRMET+pho0+bin21);
        BFI->FilterRegions( "CRLep02", nLep1+CRLepDxySig+CRMET+pho0+bin02);
        BFI->FilterRegions( "CRLep12", nLep1+CRLepDxySig+CRMET+pho0+bin12);
        BFI->FilterRegions( "CRLep22", nLep1+CRLepDxySig+CRMET+pho0+bin22);
       
       	BFI->CreateBin("CRHad00");
        BFI->CreateBin("CRHad10");
        BFI->CreateBin("CRHad20");
        BFI->CreateBin("CRHad01");
        BFI->CreateBin("CRHad11");
        BFI->CreateBin("CRHad21");
	BFI->CreateBin("CRHad02");
        BFI->CreateBin("CRHad12");
        BFI->CreateBin("CRHad22");

	BFI->CreateBin("CRLep00");
        BFI->CreateBin("CRLep10");
        BFI->CreateBin("CRLep20");
        BFI->CreateBin("CRLep01");
        BFI->CreateBin("CRLep11");
        BFI->CreateBin("CRLep21");
        BFI->CreateBin("CRLep02");
        BFI->CreateBin("CRLep12");
        BFI->CreateBin("CRLep22");

/*	
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
*/	

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

	std::string outputJSON = "test_9binCR_SV_noMC.json";	
	JSONFactory* json = new JSONFactory(BFI->analysisbins);
	json->WriteJSON("./json/"+outputJSON);
}
