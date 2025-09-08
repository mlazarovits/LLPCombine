#include "SampleTool.h"



SampleTool::SampleTool(){

	string pathPrefix = "/uscms/home/jaking/nobackup/llpana_skims/";
	MasterDict["Wjets"] = {pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-100To200_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-1200To2500_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-200To400_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-2500ToInf_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-400To600_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-600To800_rjrskim_v36.root",
				pathPrefix+"WJets_R18_SVIPM100_v31_WJetsToLNu_HT-800To1200_rjrskim_v36.root"};		
			

	MasterDict["Zjets"] = {pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-100To200_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-1200To2500_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-200To400_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-2500ToInf_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-400To600_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-600To800_rjrskim_v36.root",
				pathPrefix+"ZJets_R18_SVIPM100_v30_ZJetsToNuNu_HT-800To1200_rjrskim_v36.root" };


	MasterDict["Top"] = {pathPrefix+"TGJets_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
						 pathPrefix+"TTGJets_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
						 pathPrefix+"TTJets_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
						 pathPrefix+"ttWJets_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
						 pathPrefix+"ttZJets_R18_IPM100_v24_LLPGskim_v24_rjrvars.root"};
						 
	MasterDict["Gjets"] = {pathPrefix+"GJets_R18_SVIPM100_v30_GJets_HT-100To200_rjrskim_v36.root",
				pathPrefix+"GJets_R18_SVIPM100_v30_GJets_HT-200To400_rjrskim_v36.root",
				pathPrefix+"GJets_R18_SVIPM100_v30_GJets_HT-400To600_rjrskim_v36.root",
				pathPrefix+"GJets_R18_SVIPM100_v30_GJets_HT-40To100_rjrskim_v36.root",
				pathPrefix+"GJets_R18_SVIPM100_v30_GJets_HT-600ToInf_rjrskim_v36.root"};
	


	MasterDict["QCD"] = {pathPrefix+"QCD_HT1000to1500_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",
						 pathPrefix+"QCD_HT100to200_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",
						 pathPrefix+"QCD_HT1500to2000_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",
						 pathPrefix+"QCD_HT2000toInf_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",
						 pathPrefix+"QCD_HT200to300_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",
						 pathPrefix+"QCD_HT300to500_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",	
						 pathPrefix+"QCD_HT500to700_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",		
						 pathPrefix+"QCD_HT50to100_R18_IPM100_v25_LLPGskim_v26_rjrvars.root",	
						 pathPrefix+"QCD_HT700to1000_R18_IPM100_v25_LLPGskim_v26_rjrvars.root"};	
						 
	MasterDict["DB"] = {pathPrefix+"WZ_R18_IPM100_v24_LLPGskim_v24_rjrvars.root"};
	
	
	MasterDict["gogoG"] = {pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-1500_mN2-500_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-250_ct0p1_rjrskim_v36.root",	
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1000_mN1-500_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-1000_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-1_ct0p1_rjrskim_v36.root",   
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-250_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1500_mN1-500_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1000_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1500_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-1_ct0p1_rjrskim_v36.root",   
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-250_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1900_mN1-500_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1000_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1500_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1900_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-1_ct0p1_rjrskim_v36.root",   
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-250_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-1950_mN1-500_ct0p1_rjrskim_v36.root", 
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-500_mN1-1_ct0p1_rjrskim_v36.root",    
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2000_mN2-500_mN1-250_ct0p1_rjrskim_v36.root",  
				pathPrefix+"SMS_SVIPM100_v30_gogoG_AODSIM_mGl-2500_mN2-1500_mN1-1000_ct0p1_rjrskim_v36.root"};

						   
	MasterDict["gogoZ"] = {	pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-1500_mN2-500_mN1-100_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p001_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-200_ct0p3_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p001_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1900_mN1-350_ct0p3_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-1950_mN1-1900_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p001_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-200_ct0p3_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p001_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p1_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_gogoZ_AODSIM_mGl-2000_mN2-400_mN1-350_ct0p3_ct0p1_rjrskim_v36.root"};
	
				   
	MasterDict["sqsqG"] = { pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-1700_mN2-1500_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-1700_mN2-300_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-1850_mN2-1650_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-1850_mN2-300_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-2000_mN2-1800_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-2000_mN2-300_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-2150_mN2-1950_mN1-100_ct0p1_rjrskim_v36.root",
				pathPrefix+"SMS_SVIPM100_v30_sqsqG_AODSIM_mGl-2150_mN2-300_mN1-100_ct0p1_rjrskim_v36.root"};
 








/* (from my local laptop build)
//initialize master dictionary
	string pathPrefix = "../RJR_NTuples/batch5/";
	MasterDict["Wjets"] = {pathPrefix+"WJetsToLNu_HT-100To200_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-1200To2500_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-200To400_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-2500ToInf_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-400To600_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-600To800_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"WJetsToLNu_HT-800To1200_R18_IPM100_v24_LLPGskim_v23_rjrvars.root"};
							
	pathPrefix = "../RJR_NTuples/batch5/";
	MasterDict["Zjets"] = {pathPrefix+"ZJetsToNuNu_HT-1200To2500_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"ZJetsToNuNu_HT-200To400_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"ZJetsToNuNu_HT-2500ToInf_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"ZJetsToNuNu_HT-400To600_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"ZJetsToNuNu_HT-600To800_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
							pathPrefix+"ZJetsToNuNu_HT-800To1200_R18_IPM100_v24_LLPGskim_v23_rjrvars.root"};
	
	pathPrefix = "../RJR_NTuples/batch5/";
	MasterDict["Top"] = {pathPrefix+"TGJets_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
						pathPrefix+"TTGJets_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
						pathPrefix+"TTJets_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
						pathPrefix+"ttWJets_R18_IPM100_v24_LLPGskim_v23_rjrvars.root",
						pathPrefix+"ttZJets_R18_IPM100_v24_LLPGskim_v23_rjrvars.root"};
	
	pathPrefix = "../RJR_NTuples/batch7/";
	MasterDict["Gjets"] = {pathPrefix+"GJets_HT-100To200_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
							pathPrefix+"GJets_HT-200To400_R18_IPM100_v25_LLPGskim_v24_rjrvars.root",
							pathPrefix+"GJets_HT-400To600_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
							pathPrefix+"GJets_HT-40To100_R18_IPM100_v24_LLPGskim_v24_rjrvars.root",
							pathPrefix+"GJets_HT-600ToInf_R18_IPM100_v24_LLPGskim_v24_rjrvars.root"};	

	pathPrefix = "../RJR_NTuples/batch6/";
	MasterDict["gogoG"] = {pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1000_mN1-1_LLPGskim_v24_rjrvars.root", 
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1900_mN1-250_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1000_mN1-250_LLPGskim_v24_rjrvars.root",   
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1900_mN1-500_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1000_mN1-500_LLPGskim_v24_rjrvars.root",  
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-1000_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1500_mN1-1000_LLPGskim_v24_rjrvars.root",  
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-1500_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1500_mN1-1_LLPGskim_v24_rjrvars.root",     
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-1900_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1500_mN1-250_LLPGskim_v24_rjrvars.root",   
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-1_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1500_mN1-500_LLPGskim_v24_rjrvars.root",   
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-250_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1900_mN1-1000_LLPGskim_v24_rjrvars.root",  
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1950_mN1-500_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1900_mN1-1500_LLPGskim_v24_rjrvars.root",  
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-500_mN1-1_LLPGskim_v24_rjrvars.root",
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-1900_mN1-1_LLPGskim_v24_rjrvars.root",     
						pathPrefix+"gogoG_Sig_IPM100_v24_SMS-GlGl_AODSIM_mGl-2000_mN2-500_mN1-250_LLPGskim_v24_rjrvars.root"};		
*/

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



