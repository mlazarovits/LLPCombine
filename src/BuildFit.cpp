#include "BuildFit.h"

ch::Categories BuildFit::BuildCats(JSONFactory* j){
	ch::Categories cats{};
	int binNum=0;
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it) {
  		//std::cout << it.key() <<"\n";
		cats.push_back( {binNum, it.key()} );
		binNum++;
	}
	return cats;
}
std::map<std::string, float> BuildFit::BuildAsimovData(JSONFactory* j){

	std::map<std::string, float> obs_rates{};
	
	//outer loop bin iterator
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
		//inner loop process iterator
		std::string binname = it.key();
		float totalBkg = 0;
		for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
			//std::cout<< it2.key()<<"\n";
			
			if( BFTool::ContainsAnySubstring( it2.key(), sigkeys)){
				continue;
			}
			else{
				//get the wnevents, index 1 of array
				json json_array = it2.value();
				//std::cout<< it2.key()<<" "<<json_array[1].get<float>()<<" "<<"\n";
				totalBkg+= json_array[1].get<float>();
			}
		}
		obs_rates[binname] = float(int(totalBkg));
		//std::cout<<"adding totalbkg: "<<binname<<" "<< float(int(totalBkg))<<"\n";
	}
	return obs_rates;	
}
std::vector<std::string> BuildFit::GetBkgProcs(JSONFactory* j){
	std::vector<std::string> bkgprocs{};

	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                //      std::cout<< it2.key()<<"\n";
                        if( BFTool::ContainsAnySubstring( it2.key(), sigkeys) || it2.key() == "data_obs" || BFTool::ContainsAnySubstring(it2.key(),datakeys)){
                                continue;
                        }
                        else{
				bkgprocs.push_back(it2.key());
			}
		}
	}
	return bkgprocs;
}
std::vector<std::string> BuildFit::GetDataProcs(JSONFactory* j){
	
	std::vector<std::string> bkgprocs{};
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                //      std::cout<< it2.key()<<"\n";
                        if(  BFTool::ContainsAnySubstring(it2.key(),datakeys) ){
                                bkgprocs.push_back(it2.key());
                        }
                }
        }//make this set unique
        std::set<std::string> my_bkg_set(bkgprocs.begin(), bkgprocs.end());
        std::vector<std::string> bkgprocsunique(my_bkg_set.begin(), my_bkg_set.end());

	return bkgprocsunique;	

}
std::vector<std::string> BuildFit::ExtractSignalDetails( std::string signalPoint){

	std::vector<std::string> splitPoint = BFTool::SplitString( signalPoint, "_");
	std::string analysis = splitPoint[0];
	std::string channel = "gamma";	
	//pad for mass?
	std::string mass = "";
	for( long unsigned int i=1; i< splitPoint.size(); i++){
		mass += splitPoint[i];
	}

	std::vector<std::string> signalDetails = {analysis, channel, mass};
	return signalDetails;

}
std::vector<std::string> BuildFit::GetBinSet( JSONFactory* j){
	std::vector<std::string> bins{};
        for (json::iterator it = j->j.begin(); it != j->j.end(); ++it) {
                //std::cout << it.key() <<"\n";
                bins.push_back(  it.key() );
        }
        return bins;

}
std::map<std::string, float> BuildFit::LoadDataProcesses(JSONFactory* j, std::vector<std::string> dataKeys){
	std::map<std::string, float> obs_rates ={};
	float obs_rate=0.;
        for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
		for(int i=0; i< dataKeys.size(); i++){
                	json json_array = j->j[binname][dataKeys[i]];
               		obs_rate += json_array[1].get<float>();
       		}
		obs_rates[binname] = obs_rate;
		obs_rate=0.;
	}
        return obs_rates;

}
std::map<std::string, float> BuildFit::LoadObservations(JSONFactory* j){
	std::map<std::string, float> obs_rates ={};
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
		json json_array = j->j[binname]["data_obs"];
            	obs_rates[binname] = json_array[1].get<float>();				 
        }
	return obs_rates;
	
}
double BuildFit::GetStatFracError(JSONFactory* j, std::string binName, std::vector<std::string> bkgprocs ){
	double fracError=0;
	double statError=0;
	double bkgYield=0;

                //assign yield to obs bin map
	for(int i=0; i<bkgprocs.size(); i++){
        	json json_array = j->j[binName][bkgprocs[i]];
               	bkgYield += json_array[1].get<float>();
		statError += json_array[2].get<float>()*json_array[2].get<float>();
	}
	if(bkgYield>0){
		fracError = std::sqrt(statError)/bkgYield;
	}
	else{
		fracError = 1.;
	}
        
        return fracError;
}
void BuildFit::BuildABCDFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir, std::vector<std::string> ABCDbins){	
	

	ch::Categories cats = BuildCats(j);
        std::cout<<"building obs rates \n";//fit should think dataprocs are mc bkgs so we can re-use the same functions
        std::map<std::string, float> obs_rates = BuildAsimovData(j);
        std::cout<<"Getting process list\n";
        std::vector<std::string> bkgprocs = GetDataProcs(j);
        std::cout<<"Parse Signal point\n";
        std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint);
        std::cout<<"Build cb objects\n";
        //cb.SetVerbosity(3);
	
	std::string A = ABCDbins[0];
        std::string B = ABCDbins[1];
        std::string C = ABCDbins[2];
        std::string D = ABCDbins[3];

	//set the fake proc rate in A to be multiplicative, not the observation
        double bkgrateA = obs_rates[B]*(obs_rates[C]/obs_rates[D]);

	
        cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
        cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
        cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);
        cb.ForEachObs([&](ch::Observation *x){
                x->set_rate(obs_rates[x->bin()]);
        });
	
        cb.ForEachProc([&j,&A,&bkgrateA,&signalPoint](ch::Process *x) {
            std::cout<<x->bin()<<" "<<x->process()<<"\n";
	   if(x->bin() == A && x->process() != signalPoint){
		x->set_rate(bkgrateA);
	   }	
	   else{
	   	json json_array = j->j[x->bin()][x->process()];
            	x->set_rate(json_array[1].get<float>());
	   }
        });

	std::vector<std::string> binset = GetBinSet(j);	
	//cb.cp().bin(binset).AddSyst(cb, "DummySys", "lnN", SystMap<>::init(1.50));		

	cb.cp().bin({B, C, D}). AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init
	 ({B}, 1.0)
	 ({C}, 1.0)
	 ({D}, 1.0)
	);
     
	//optional pin rates lognormals
	cb.cp().bin({B}).AddSyst(cb, "lnN_"+B, "lnN", SystMap<>::init(1.001));
	cb.cp().bin({C}).AddSyst(cb, "lnN_"+C, "lnN", SystMap<>::init(1.50));
	cb.cp().bin({D}).AddSyst(cb, "lnN_"+D, "lnN", SystMap<>::init(1.50));


	//create the A prediction bin
	 cb.cp().bin({A}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
          ("(@0*@1/@2)", "scale_"+B +",scale_"+C+ ",scale_"+D)
      );
	cb.PrintAll();
        cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");

}
void BuildFit::BuildAsimovFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir){
	ch::Categories cats = BuildCats(j);
	std::cout<<"building obs rates \n";
	std::map<std::string, float> obs_rates = BuildAsimovData(j);
	std::cout<<"Getting process list\n";
	std::vector<std::string> bkgprocs = GetBkgProcs(j);
	std::cout<<"Parse Signal point\n";
	std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint);
	std::cout<<"Build cb objects\n";
	//cb.SetVerbosity(3);
	cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
	cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
	cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);
	cb.ForEachObs([&](ch::Observation *x){
		x->set_rate(obs_rates[x->bin()]);
	});
	cb.ForEachProc([&j](ch::Process *x) {
	    //std::cout<<x->bin()<<" "<<x->process()<<"\n";
	    json json_array = j->j[x->bin()][x->process()];
	    x->set_rate(json_array[1].get<float>());
	});

	std::vector<std::string> binset = GetBinSet(j);
	cb.cp().bin(binset).AddSyst(cb, "DummySys", "lnN", SystMap<>::init(1.10));
	

      
	//cb.PrintAll();
	cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");

}
void BuildFit::BuildMultiChannel9bin(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap){
	ch::Categories cats = BuildCats(j);
        std::cout<<"building obs rates \n";
        std::map<std::string, float> obs_rates = LoadDataProcesses(j, {"MET18"});
        std::cout<<"Getting process list\n";
        std::vector<std::string> bkgprocs = GetDataProcs(j);
        std::cout<<"Parse Signal point\n";
        std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint );
        std::cout<<"Build cb objects\n";

        cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
        cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
        cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);

        cb.ForEachObs([&](ch::Observation *x){
            if( obs_rates[x->bin()]==0){
                x->set_rate(1e-8);
           }
           else{
            x->set_rate(obs_rates[x->bin()]);
           }
        });
	//take channel 1 as the anchor channel, enforce expectation onto other channels
	cb.ForEachProc([&j](ch::Process *x) {
            std::cout<<x->bin()<<" "<<x->process()<<"\n";
            json json_array = j->j[x->bin()][x->process()];
            float temprate = json_array[1].get<float>();
            std::string c1 = "Ch1CRHad"; //hardcode anchor channel for now
	    std::string c2 = "Ch2CRHad";
            std::string c3 = "Ch3CRLep";
            size_t foundPos = x->bin().find(c2);
	    size_t foundPos2 = x->bin().find(c3); //very bad, need better solution to be more general
            if( foundPos != std::string::npos){
                std::string mirrorbin = x->bin();
                mirrorbin.replace(foundPos, c2.length(), c1);
                json_array = j->j[mirrorbin][x->process()];
                temprate = json_array[1].get<float>();
                if(temprate==0){
                        x->set_rate(1e-8);
                }
                else{
                        x->set_rate(temprate);
                }
            }
            else{
                if(temprate==0){
                        x->set_rate(1e-8);
                }
                else{
                        x->set_rate(temprate);
                }
            }
	    if( foundPos2 != std::string::npos){
                std::string mirrorbin = x->bin();
                mirrorbin.replace(foundPos2, c3.length(), c1);
                json_array = j->j[mirrorbin][x->process()];
                temprate = json_array[1].get<float>();
                if(temprate==0){
                        x->set_rate(1e-8);
                }
                else{
                        x->set_rate(temprate);
                }
            }
            else{
                if(temprate==0){
                        x->set_rate(1e-8);
                }
                else{
                        x->set_rate(temprate);
                }
            }
	});
	std::vector<std::string> bincoords = { "00","10","20", "01","11","21","02","12","22"};
        std::string ch1 = "Ch1CRHad";
        std::string ch2 = "Ch2CRHad";
	std::string ch3 = "Ch3CRHad";
        for(int i=0; i<bincoords.size(); i++){
                cb.cp().bin({ch1+bincoords[i], ch2+bincoords[i]}).AddSyst(cb, "c1c2binShape"+bincoords[i], "lnN", SystMap<>::init(1.05));
		cb.cp().bin({ch1+bincoords[i], ch3+bincoords[i]}).AddSyst(cb, "c1c3binShape"+bincoords[i], "lnN", SystMap<>::init(1.05));

        }
        //make ch2 normalization
        std::vector<std::string> ch2bins = channelMap["ch2"];
	std::vector<std::string> ch3bins = channelMap["ch3"];
        cb.cp().bin(ch2bins).AddSyst(cb, "c2lepNorm", "rateParam", SystMap<>::init(0.085));
	cb.cp().bin(ch3bins).AddSyst(cb, "c3lepNorm", "rateParam", SystMap<>::init(0.44));
        cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");

	
	
}
void BuildFit::Build9binFitData(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap){

	ch::Categories cats = BuildCats(j);
	std::cout<<"building obs rates \n";
	std::map<std::string, float> obs_rates = LoadDataProcesses(j, {"MET18"});
	std::cout<<"Getting process list\n";
	std::vector<std::string> bkgprocs = GetDataProcs(j);
	std::cout<<"Parse Signal point\n";
	std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint );
	std::cout<<"Build cb objects\n";

	cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
	cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
	cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);
	
	cb.ForEachObs([&](ch::Observation *x){
            if( obs_rates[x->bin()]==0){
                x->set_rate(1e-8);
           }
           else{
            x->set_rate(obs_rates[x->bin()]);
           }
        });
	//use ch1 to set expectation in ch2
        cb.ForEachProc([&j](ch::Process *x) {
            std::cout<<x->bin()<<" "<<x->process()<<"\n";
            json json_array = j->j[x->bin()][x->process()];
            float temprate = json_array[1].get<float>();
	    std::string c1 = "Had";
	    std::string c2 = "Lep";
	    size_t foundPos = x->bin().find(c2);
	    if( foundPos != std::string::npos){
	    	std::string mirrorbin = x->bin();
	    	mirrorbin.replace(foundPos, c2.length(), c1);
		json_array = j->j[mirrorbin][x->process()];
		temprate = json_array[1].get<float>();
            	if(temprate==0){
               	 	x->set_rate(1e-8);
            	}
           	else{
                 	x->set_rate(temprate);
            	}
	    }
	    else{
	    	if(temprate==0){
                 	x->set_rate(1e-8);
                }
                else{
                 	x->set_rate(temprate);
                }
	    }
            //x->set_rate(json_array[1].get<float>());
        });

	/*
    	std::string A = "CRHad00";
	std::string B = "CRHad10";
	std::string C = "CRHad01";
	std::string D = "CRHad11";	
	//do an internal abc on the anchor channel
	cb.cp().bin({B, C, D}).AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init
         ({B}, 1.0)
         ({C}, 1.0)
         ({D}, 1.0)
        );
	cb.cp().bin({A}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
          ("(@0*@1/@2)", "scale_"+B +",scale_"+C+ ",scale_"+D)
        );
	*/
	//map each 9 bin together.. could do small lnN or exact rate
	std::vector<std::string> bincoords = { "00","10","20", "01","11","21","02","12","22"};
	std::string ch1 = "CRHad";
        std::string ch2 = "CRLep";
	for(int i=0; i<bincoords.size(); i++){
		cb.cp().bin({ch1+bincoords[i], ch2+bincoords[i]}).AddSyst(cb, "binShape"+bincoords[i], "lnN", SystMap<>::init(1.05));			
	}
	//make ch2 normalization
	std::vector<std::string> ch2bins = channelMap["chLep1"]; 
	cb.cp().bin(ch2bins).AddSyst(cb, "lepNorm", "rateParam", SystMap<>::init(0.3));
	cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");
	



}
void BuildFit::Build9binFitMC(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap){

	ch::Categories cats = BuildCats(j);
        std::cout<<"building obs rates \n";
        std::map<std::string, float> obs_rates = LoadDataProcesses(j, {"MET18"});
        std::cout<<"Getting process list\n";
        std::vector<std::string> bkgprocs = GetBkgProcs(j);
        std::cout<<"Parse Signal point\n";
        std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint);
        std::cout<<"Build cb objects\n";

	cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
        cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
        cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);
        cb.ForEachObs([&](ch::Observation *x){
            if( obs_rates[x->bin()]==0){
		x->set_rate(1e-8);
	   }  
	   else{
	    x->set_rate(obs_rates[x->bin()]);
	   }
        });
        cb.ForEachProc([&j](ch::Process *x) {
            std::cout<<x->bin()<<" "<<x->process()<<"\n";
            json json_array = j->j[x->bin()][x->process()];
	    float temprate = json_array[1].get<float>();
	    if(temprate==0){
		x->set_rate(1e-8);
	    }
	    else{
		x->set_rate(temprate);
	    }
            //x->set_rate(json_array[1].get<float>());
        });
	//map everything together
	//global rate for lumi normalization
	std::vector<std::string> binset = GetBinSet(j);
       	cb.cp().bin(binset).AddSyst(cb, "LumiScale", "rateParam", SystMap<>::init(1.0));

	//map each bin together
	std::vector<std::string> chHad1 = channelMap["chHad1"];
	std::vector<std::string> chLep1 = channelMap["chLep1"];
	for(int i=0; i<chHad1.size(); i++){// frac error based on stat error
		double fracError= GetStatFracError(j, chHad1[i], bkgprocs );
		cb.cp().bin({chHad1[i], chLep1[i]}).AddSyst(cb, "binShape"+std::to_string(i), "lnN", SystMap<>::init(1+fracError));
		std::cout<<"lnN for "+chHad1[i]<<" "<<1+fracError<<"\n";
	}
	//add unpenalized normalization for chLep
	cb.cp().bin(chLep1).AddSyst(cb, "LepNorm", "rateParam", SystMap<>::init(1.0));	
	cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");

}
void BuildFit::BuildPseudoShapeTemplateFit(JSONFactory* j, JSONFactory* jup, JSONFactory* jdn,  std::string signalPoint, std::string datacard_dir, channelmap channelMap){	

	std::cout<<" using channel map:\n";
	for (const auto& pair : channelMap) {
        std::cout << "Channel: " << pair.first << std::endl;
        std::cout << "Bins: ";
        for (const std::string& b : pair.second) {
            std::cout << b << " ";
        }
        std::cout << std::endl;
    	}
	ch::Categories cats = BuildCats(j);
	std::map<std::string, float> obs_rates = LoadObservations(j);
        std::cout<<"Getting process list\n";
        std::vector<std::string> bkgprocs = GetBkgProcs(j);
        std::cout<<"Parse Signal point\n";
        std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint);
        std::cout<<"Build cb objects\n";
	cb.AddObservations({"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, cats);
        cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6TeV"}, {signalDetails[1]}, bkgprocs, cats, false);
        //cb.AddProcesses(   {signalDetails[2]}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);
        cb.AddProcesses(   {"*"}, {signalDetails[0]}, {"13.6Tev"}, {signalDetails[1]}, {signalPoint}, cats, true);

	cb.ForEachObs([&](ch::Observation *x){
                x->set_rate(obs_rates[x->bin()]);
		//TH1F shape("htemp","temp",1,0,1);
	        //shape.SetBinContent(1, obs_rates[x->bin()]);
            	//shape.SetBinError(1, std::sqrt(obs_rates[x->bin()]));
            	//x->set_shape( shape, true );

        });
        cb.ForEachProc([&j](ch::Process *x) {
            //std::cout<<x->bin()<<" "<<x->process()<<"\n";
            json json_array = j->j[x->bin()][x->process()];
            x->set_rate(json_array[1].get<float>());
	    //set the shapes for bbb mode
	    //TH1F shape("htemp","temp",1,0,1);
	    //shape.SetBinContent(1, json_array[1].get<float>());
	    //shape.SetBinError(1, json_array[2].get<float>()); 
	   // x->set_shape( shape, true );
        });


	//try nuisance per shape bin with up down as asymmetric lnN
        std::vector<std::string> binset = GetBinSet(j);
        
	//cb.cp().bin(binset).AddSyst(cb, "DummySys", "lnN", SystMap<>::init(1.10));
	for(int i=0; i<binset.size(); i++){
	 	
		break;	
		json json_array_nom = j->j[binset[i]]["bkg"];
		json json_array_up = jup->j[binset[i]]["bkg"];
		json json_array_dn = jdn->j[binset[i]]["bkg"];
		float valnom = json_array_nom[1].get<float>();
		float valup  = json_array_up[1].get<float>();
		float valdn  = json_array_dn[1].get<float>();
                float fracError = (valup-valdn)/valnom;
		float fracDown = std::abs((valnom-valdn)/valnom);
		float fracUp = std::abs((valup-valnom)/valnom);
		std::cout<<"adding bin NP "<<binset[i]<<" "<<1+fracError<<" "<< fracDown<<" "<<fracUp<<"\n"; 
		cb.cp().bin({binset[i]}).AddSyst(cb,"lnN"+binset[i],"lnN", SystMap<ch::syst::process>::init
				({"bkg"}, 1+fracUp)
				({"bkg"}, 1-fracDown));
	}
	//do stat by hand with lnn
	for( int i=0; i<binset.size(); i++){		
		json json_array_nom = j->j[binset[i]]["bkg"];
                float valnom = json_array_nom[1].get<float>();
		float staterr = json_array_nom[2].get<float>();
		//float fracError = staterr/valnom;
		//combine errors mode
		json json_array_up = jup->j[binset[i]]["bkg"];
                json json_array_dn = jdn->j[binset[i]]["bkg"];
		float valup  = json_array_up[1].get<float>();
                float valdn  = json_array_dn[1].get<float>();
                float diffDown = std::abs((valnom-valdn));
                float diffUp = std::abs((valup-valnom));
		float shapeerr = std::max(diffDown,diffUp);
		float totalerr = std::sqrt( staterr*staterr + shapeerr*shapeerr );
		//combine error fractions?
		float fracError= totalerr/valnom;
		std::cout<<"adding bin NP "<<binset[i]<<" "<<1+fracError<<"\n";
                cb.cp().bin({binset[i]}).AddSyst(cb,"lnN"+binset[i],"lnN", SystMap<>::init(1+fracError));
	}

	//do bbb
	/*
	auto bbb = ch::BinByBinFactory()
                .SetAddThreshold(0.1)
                .SetFixNorm(true);

        bbb.AddBinByBin(cb.cp().backgrounds(), cb);
	*/
	//cb.cp().SetAutoMCStats(cb, 0.1);

        //cb.PrintAll();
        cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");//,datacard_dir+"/"+signalPoint+"/bbbshapes.root");
		

}
//debug and dev function - put here to make compiling easy
void StatUncertaintyTesting(){
	
	//runs over combine paper datacard and adds mcstats or binbybin stat uncertainties
	//mcstats testing
        ch::CombineHarvester cb;

        // Call the `ParseDatacard` method specifying the path to the text file and the additional
        // metadata we want to associate with it
        cb.ParseDatacard("datacards_test1_MCStats/shape_template_test/datacard-2-template-analysis.txt", "shapetest", "", "", 1, "");
        cb.cp().process({"background"}).AddSyst(cb, "alpha", "shape", SystMap<>::init(1.00));
        cb.cp().process({"signal"}).AddSyst(cb, "sigma", "shape", SystMap<>::init(0.5));
        // Print the Observation, Process and Systematic entries to the screen
 	//cb.PrintObs().PrintProcs().PrintSysts();
        cb.cp().backgrounds().ExtractShapes("datacards_test1_MCStats/shape_template_test/template-analysis-datacard-input.root","$PROCESS","$PROCESS_$SYSTEMATIC");
        cb.cp().signals().ExtractShapes("datacards_test1_MCStats/shape_template_test/template-analysis-datacard-input.root","$PROCESS","$PROCESS_$SYSTEMATIC");
	//cb.PrintObs().PrintProcs().PrintSysts();
        //do the mcstats
        //cb.cp().SetAutoMCStats(cb, -1.);
        //cb.PrintAll();
        //cb.WriteDatacard("datacards_test1_MCStats/shape_template_test/datacard-2-template-analysis_autoMCstat.txt","datacards_test1_MCStats/shape_template_test/testShapesOutput.root");

          auto bbb = ch::BinByBinFactory()
                .SetAddThreshold(0.1)
                .SetFixNorm(true);

        bbb.AddBinByBin(cb.cp().backgrounds(), cb);
        cb.PrintAll();
        cb.WriteDatacard("datacards_test1_MCStats/shape_template_test/datacard-2-template-analysis_bbbstat.txt","datacards_test1_MCStats/shape_template_test/testShapesOutput_bbb.root");

}

