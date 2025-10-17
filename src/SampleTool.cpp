#include "SampleTool.h"



SampleTool::SampleTool(){

	string pathPrefix = "/uscms/home/jaking/nobackup/llpana_skims/";
	MasterDict["Wjets"] = {pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-100To200_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-1200To2500_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-200To400_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-2500ToInf_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-400To600_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-600To800_rjrskim_v38.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-800To1200_rjrskim_v38.root"};		
			

	MasterDict["Zjets"] = {pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-100To200_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-1200To2500_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-200To400_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-2500ToInf_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-400To600_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-600To800_rjrskim_v38.root",
				pathPrefix+"ZJets_R18_SVIPM100_v31_ZJetsToNuNu_HT-800To1200_rjrskim_v38.root"};


	MasterDict["Top"] = {pathPrefix+"TTXJets_R18_SVIPM100_v31_TGJets_rjrskim_v38.root",
				pathPrefix+"TTXJets_R18_SVIPM100_v31_TTGJets_rjrskim_v38.root",
				pathPrefix+"TTXJets_R18_SVIPM100_v31_TTJets_rjrskim_v38.root",
				pathPrefix+"TTXJets_R18_SVIPM100_v31_ttWJets_rjrskim_v38.root",
				pathPrefix+"TTXJets_R18_SVIPM100_v31_ttZJets_rjrskim_v38.root"};
						 
	MasterDict["Gjets"] = {pathPrefix+"GJets_R18_SVIPM100_v31_GJets_HT-100To200_rjrskim_v38.root",
				pathPrefix+"GJets_R18_SVIPM100_v31_GJets_HT-200To400_rjrskim_v38.root",
				pathPrefix+"GJets_R18_SVIPM100_v31_GJets_HT-400To600_rjrskim_v38.root",
				pathPrefix+"GJets_R18_SVIPM100_v31_GJets_HT-40To100_rjrskim_v38.root",
				pathPrefix+"GJets_R18_SVIPM100_v31_GJets_HT-600ToInf_rjrskim_v38.root"};
	


	MasterDict["QCD"] = {pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT1000to1500_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT100to200_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT1500to2000_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT2000toInf_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT200to300_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT300to500_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT500to700_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT50to100_rjrskim_v38.root",
				pathPrefix+"QCD_R18_SVIPM100_v31_QCD_HT700to1000_rjrskim_v38.root"};	
						 
	MasterDict["DB"] = {pathPrefix+"WZ_R18_IPM100_v24_LLPGskim_v24_rjrvars.root"};

	MasterDict["Box"] = {pathPrefix+"DiPJBox_R18_SVIPM100_v31_DiPhotonJetsBox_M40_80-sherpa_rjrskim_v38.root",
				pathPrefix+"DiPJBox_R18_SVIPM100_v31_DiPhotonJetsBox_MGG-0to40_rjrskim_v38.root",
				pathPrefix+"DiPJBox_R18_SVIPM100_v31_DiPhotonJetsBox_MGG-80toInf_rjrskim_v38.root"};	
	
	MasterDict["gogoG"] = {pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-1500_mN2-500_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-1_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-250_ct0p1_rjrskim_v38.root",	
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-500_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-1000_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-1_ct0p1_rjrskim_v38.root",   
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-250_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-500_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1000_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1500_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1_ct0p1_rjrskim_v38.root",   
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-250_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-500_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1000_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1500_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1900_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1_ct0p1_rjrskim_v38.root",   
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-250_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-500_ct0p1_rjrskim_v38.root", 
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-500_mN1-1_ct0p1_rjrskim_v38.root",    
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2000_mN2-500_mN1-250_ct0p1_rjrskim_v38.root",  
				pathPrefix+"SMS_SVIPM100_v31_gogoG_AODSIM_mGl-2500_mN2-1500_mN1-1000_ct0p1_rjrskim_v38.root"};

						   
	MasterDict["gogoZ"] = {	pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-1500_mN2-500_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p001_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p3_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p001_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p3_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-1950_mN1-1900_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p001_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p3_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p001_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p3_rjrskim_v38.root"};
	
				   
	MasterDict["sqsqG"] = { pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-1700_mN2-1500_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-1700_mN2-300_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-1850_mN2-1650_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-1850_mN2-300_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-2000_mN2-1800_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-2000_mN2-300_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-2150_mN2-1950_mN1-100_ct0p1_rjrskim_v38.root",
				pathPrefix+"SMS_SVIPM100_v31_sqsqG_AODSIM_mGl-2150_mN2-300_mN1-100_ct0p1_rjrskim_v38.root"};
 



	//data skims
	//need new data skim
	//pathPrefix = pathPrefix+"rjrskims_v37/";
	MasterDict["DisplacedJet18"] = { pathPrefix+"DisJet_R18_SVIPM100_v31_DisplacedJet_AOD_Run2018D_rjrskim_v38.root"};
	//MasterDict["DisplacedJet17"] = { pathPrefix+"DisJet_R18_SVIPM100_v31_DisplacedJet_AOD_Run2018D-15Feb2022_UL2018-v1_v29_rjrskim_v37.root"};
	
	MasterDict["MET18"] = { pathPrefix+"MET_R18_SVIPM100_v31_MET_AOD_Run2018A_rjrskim_v38.root",
				pathPrefix+"MET_R18_SVIPM100_v31_MET_AOD_Run2018B_rjrskim_v38.root",
				pathPrefix+"MET_R18_SVIPM100_v31_MET_AOD_Run2018C_rjrskim_v38.root"};


}
void SampleTool::LoadBkgs( stringlist& bkglist ){
	for( long unsigned int i=0; i<bkglist.size(); i++){
		//check if background exists
		if( MasterDict.count(bkglist[i]) == 0 ){
			std::cout<<"Bkg: "<<bkglist[i]<<" not found ... skipping ...\n";
			continue;
		} 
		BkgDict[bkglist[i]] = MasterDict[bkglist[i]];		
	}
}
void SampleTool::LoadData( stringlist& datalist ){
        for( long unsigned int i=0; i<datalist.size(); i++){
                //check if background exists
                if( MasterDict.count(datalist[i]) == 0 ){
                        std::cout<<"Data: "<<datalist[i]<<" not found ... skipping ...\n";
                        continue;
                }
                DataDict[datalist[i]] = MasterDict[datalist[i]];
        }
}
void SampleTool::LoadSigs( stringlist& siglist ){

	for( long unsigned int i=0; i<siglist.size(); i++){
		if( MasterDict.count(siglist[i]) == 0 ){
			std::cout<<"Sig: "<<siglist[i]<<" not found ... skipping ...\n";
			continue;
		}
		SigDict[siglist[i]] = MasterDict[siglist[i]];
	}
	//build signal keys
	stringlist s_strings{};
	for(long unsigned int i=0; i<siglist.size(); i++){
		std::vector< std::string > keylist{};
		s_strings = SigDict[siglist[i]];
		for( long unsigned int j=0; j< s_strings.size(); j++){
			//keylist.push_back( GetSignalTokens( s_strings[j] ) );
			SignalKeys.push_back( BFTool::GetSignalTokens( s_strings[j] ) );
		}
	}
}
void SampleTool::PrintDict( map<string,stringlist>& d ){
	for(auto it = d.cbegin(); it != d.cend(); ++it){
    	std::cout << "key:"<< it->first << ":\n";
    	stringlist str = it->second;
    	for (std::vector<string>::iterator it2 = str.begin(); it2 != str.end(); ++it2) {
        std::cout << *it2 << " \n";
    	}
    	
	}
	std::cout<<"\n";
	
}

void SampleTool::PrintKeys( stringlist sl ){
	
	for( long unsigned int i = 0; i<sl.size(); i++){
		std::cout<<sl[i]<<"\n";
	}

}



