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
                        if( BFTool::ContainsAnySubstring( it2.key(), sigkeys)){
                                continue;
                        }
                        else{
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
void BuildFit::BuildABCDFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir, std::vector<std::string> ABCDbins){	
	

	ch::Categories cats = BuildCats(j);
        std::cout<<"building obs rates \n";//fit should think dataprocs are mc bkgs so we can re-use the same functions
        std::map<std::string, float> obs_rates = BuildAsimovData(j);
        std::cout<<"Getting process list\n";
        std::vector<std::string> bkgprocs = GetBkgProcs(j);
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



