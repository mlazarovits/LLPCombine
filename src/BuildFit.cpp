#include "BuildFit.h"
#include <iostream>
#include <filesystem>
#include <sstream>
using std::cout;
using std::endl;

double BuildFit::GetYieldValue(const string& bin, const string& proc, int index, const string& context) const{
	if(!_yields.is_object()){
		throw std::runtime_error("BF JSON error in " + context + ": top-level JSON is " + string(_yields.type_name()) + ", expected object keyed by bin");
	}
	if(!_yields.contains(bin)){
		throw std::runtime_error("BF JSON error in " + context + ": missing bin '" + bin + "'");
	}
	const json& bin_json = _yields.at(bin);
	if(!bin_json.is_object()){
		throw std::runtime_error("BF JSON error in " + context + ": bin '" + bin + "' is " + string(bin_json.type_name()) + ", expected object keyed by process");
	}
	if(!bin_json.contains(proc)){
		throw std::runtime_error("BF JSON error in " + context + ": missing process '" + proc + "' in bin '" + bin + "'");
	}
	const json& proc_json = bin_json.at(proc);
	if(!proc_json.is_array()){
		throw std::runtime_error("BF JSON error in " + context + ": bin '" + bin + "', process '" + proc + "' is " + string(proc_json.type_name()) + ", expected [unweighted, weighted, staterr]");
	}
	if(index < 0 || index >= static_cast<int>(proc_json.size())){
		std::ostringstream msg;
		msg << "BF JSON error in " << context << ": bin '" << bin << "', process '" << proc << "' has "
		    << proc_json.size() << " yield entries, cannot read index " << index;
		throw std::runtime_error(msg.str());
	}
	const json& value = proc_json.at(index);
	if(!value.is_number()){
		std::ostringstream msg;
		msg << "BF JSON error in " << context << ": bin '" << bin << "', process '" << proc
		    << "', yield index " << index << " is " << value.type_name()
		    << " with value " << value.dump() << ", expected number";
		throw std::runtime_error(msg.str());
	}
	return value.get<double>();
}

//takes input fit config and JSONFactor as inputs
BuildFit::BuildFit(string infile){
	if(infile == ""){
		return;
	}
	//check if file exists
	if(!std::filesystem::exists(infile)){
		cout << "File " << infile << " does not exist" << endl;
		return;
	}
	YAML::Node base = YAML::LoadFile(infile);
	if(base["fitname"])
		_fitname = base["fitname"].as<string>();
	if(base["shape_transfer_fit"]){
		if(base["shape_transfer_fit"]["bin_association"])
        		_shape_bin_ass = base["shape_transfer_fit"]["bin_association"].as<map<string,vector<string>>>();
		if(base["shape_transfer_fit"]["channel_association"])
        		_shape_ch_ass =  base["shape_transfer_fit"]["channel_association"].as<map<string,vector<string>>>();
	}
	if(base["ABCD_fit"]){
		if(base["ABCD_fit"]["bin_association"])
			_abcd_bin_ass = base["ABCD_fit"]["bin_association"].as<map<string,vector<string>>>();
		if(base["ABCD_fit"]["channel_association"])
			_abcd_ch_ass =  base["ABCD_fit"]["channel_association"].as<map<string,vector<string>>>();
	}
	if(base["systematics"]){
		YAML::Node systs = base["systematics"];
		//fill yamlSys classes - nested map
		for(auto it = systs.begin(); it != systs.end(); it++){
			_systs.push_back(yamlSys(it->second));
		}
	}

	//set anchor bins in all shape transfer channel mappings
	for(auto it = _shape_ch_ass.begin(); it != _shape_ch_ass.end(); it++){
		//loop through all bins of buoy channel - anchor bin is the one with CR in the name for both anchor and buoy channels
		vector<string> buoy_chs = it->second;
		string anchor_ch = it->first;
		for(auto buoy_ch : buoy_chs){ 
			vector<string> buoy_bins = _shape_bin_ass[buoy_ch];
			for(auto buoy_bin : buoy_bins){
				if(buoy_bin.find("CR") != string::npos){
					string bin = getBinIdx(buoy_bin);
					_shape_anchor_bins[buoy_ch] = bin;
					_shape_anchor_bins[anchor_ch] = bin;
					break;
				}

			}

		}
	}
	for(auto it = _shape_anchor_bins.begin(); it != _shape_anchor_bins.end(); it++){
		string ch = it->first;
		string bin = it->second;
		cout << "channel " << ch << " has anchor bin " << bin << endl;
	}

	//make set of all available bins
	for(auto it = _shape_bin_ass.begin(); it != _shape_bin_ass.end(); it++){
		vector<string> bins = it->second;
		for(auto bin : bins) _bins_superset.insert(bin);
		for(auto bin : bins) _bins_superset_shape.insert(bin);
	}
	for(auto it = _abcd_bin_ass.begin(); it != _abcd_bin_ass.end(); it++){
		vector<string> bins = it->second;
		for(auto bin : bins) _bins_superset.insert(bin);
		for(auto bin : bins) _bins_superset_abcd.insert(bin);
	}

	if(base["asimov"].as<bool>())
		_asimov = true;
	else
		_asimov = false;
	if(base["datadriven"].as<bool>())
		_datadriven = true;
	else
		_datadriven = false;



}


//builds categories, makes asimov obs if specified, etc
void BuildFit::PrepFit(JSONFactory* j, string signalPoint, vector<string> datakeys){
	_yields = j->j;
	//build cats per fit-type call
	BuildCats(j);
cout << "built cats" << endl;	
	if(_asimov && !_datadriven){
		cout << "building asimov data" << endl;
		BuildAsimovData(j);
	}
	else{
		if(datakeys.size() == 0){
			cout << "Loading obs" << endl;
			LoadObservations(j);
		}
		else{
			cout << "loading data processes" << endl;
			LoadDataProcesses(j, datakeys);
		}
	}
	if(_datadriven)
		GetDataProcs(j);
	else
		GetBkgProcs(j);

	//sums over all bkg processes to get 1 bkg process
	//there only is 1 bkg process bc the bkg estimation will be done in data
	sumBkgs();
	std::cout<<"Parsing signal point " << signalPoint << endl;
	_signalPoint = signalPoint;
	ExtractSignalDetails( _signalPoint);
	SetSignalRates();
}


void BuildFit::SetObservations(){
	if(_obs_rates.size() < 1){
		cout << "Error: please call PrepFit(JSONFactory* j, string signalPoint, vector<string> datakeys = {}) to initialize observations" << endl;
		return;
	}
	//set observations based on which bins were specified in fit config	
	cb.AddObservations({"*"}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, _cats);
        cb.ForEachObs([&](ch::Observation *x){
            x->set_rate(_obs_rates[x->bin()]);
	    //cout << x->bin() << " " << x->process() << " " << x->rate() << endl;
        });

}

void BuildFit::SetSignalRates(){
        cb.AddProcesses(   {_signalDetails[2]}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, {_signalPoint}, _cats, true);
	//set signal rates
	cb.ForEachProc([&](ch::Process *x){
		if(x->process() == _signalPoint){
			double yield = GetYieldValue(x->bin(), x->process(), 1, "SetSignalRates");
			if(yield == 0)
				yield = 1e-8;
			x->set_rate(yield);
		}
	});

}

//create 1 bkg process that will behave like the bkg process from data in the datacard
void BuildFit::sumBkgs(){
	string newproc = "bkg";
	for(auto bin : _bins_superset){
		double wt_yield = 0;
		double unwt_yield = 0;
		double sq_err = 0;
		for(auto proc : _bkgprocs){
			wt_yield += GetYieldValue(bin, proc, 1, "sumBkgs");
			unwt_yield += GetYieldValue(bin, proc, 0, "sumBkgs");
			double err = GetYieldValue(bin, proc, 2, "sumBkgs");
			sq_err += err*err;
		}
		//add to json array
		_yields[bin][newproc] = {unwt_yield, wt_yield, sqrt(sq_err)}; 
	}
}

//channel name convention goes
//Ch#IDBin where Bin = xy coord in MsRs plane
void BuildFit::BuildShapeTransferFit(){
	cout << "Building Shape Transfer Fit" << endl;
	if(_shape_ch_ass.size() < 1){
		cout << "No channel association specified for shape transfer fit. This fit config will not be written. Returning." << endl;
		return;
	}
	//take channel 1 as the anchor channel, enforce expectation onto other channels
	//for channel connected to anchor channel, set initial value to value of non-anchor channel bin to bin in anchor channel
	//this way, the rate parameter connecting these channels is initialized to the ratio of the CR-like (signal depleted, high stats) bins across the anchor and non-anchor channel
	string proc = "bkg"; 
	for(auto chit = _shape_ch_ass.begin(); chit != _shape_ch_ass.end(); chit++){
		//get anchor channel as specified from fit config
		string anchor_ch = chit->first;
		vector<string> buoy_chs = chit->second;
		vector<string> anchor_bins = _shape_bin_ass[anchor_ch];
		//set yields in every bin of the anchor channel to their nominal value
		for(int b = 0; b < (int)anchor_bins.size(); b++){
			double anchor_rate = GetYieldValue(anchor_bins[b], proc, 1, "BuildShapeTransferFit anchor rate");
			cout << "anchor_bin " << anchor_bins[b] << " proc " << proc << " rate " << anchor_rate << endl;
			//set yield
			ch::Process anchorproc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],proc,make_pair(_invcats[anchor_bins[b]],anchor_bins[b]), false, anchor_rate);
			cb.InsertProcess(anchorproc);
			//get bin indices - should be last two characters based on naming convention
			string bin = getBinIdx(anchor_bins[b]);
			//set yield in buoy channels to that of connected anchor channel for this process
			for(int i = 0; i < (int)buoy_chs.size(); i++){
				vector<string> buoy_bins = _shape_bin_ass[buoy_chs[i]];
				//make sure bin configuration for anchor channel matches that for buoy channel
				if(buoy_bins.size() != anchor_bins.size()){
					cout << "Buoy channel " << buoy_chs[i] << " has " << buoy_bins.size() << " bins which does not map to anchor channel " << anchor_ch << " which has " << anchor_bins.size() << ". Skipping." << endl;
					continue;
				}
				string buoy_bin;
				if(bin == _shape_anchor_bins[buoy_chs[i]]){
					cout << "is buoy ch " << buoy_chs[i] << " bin " << bin << endl;
					buoy_bin = GetBuoyBin(buoy_chs[i]);
				}
				else{
					buoy_bin = buoy_chs[i]+bin;
				}
				//make sure anchor channel bin exists in buoy channel
				if(find(buoy_bins.begin(), buoy_bins.end(), buoy_bin) == buoy_bins.end())
					continue;
				//match bin indices (assuming that bins have been defined identically)
				//loop through bins in this channel
				cout << " buoy_bin " << buoy_bin << " proc " << proc << " rate " << anchor_rate << endl;
				ch::Process buoyproc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],proc,make_pair(_invcats[buoy_bin],buoy_bin), false, anchor_rate);
				cb.InsertProcess(buoyproc);
			}
		}
	}
	
	//cb.PrintProcs();
	//add systematics to tie bins and channels together
	for(auto chit = _shape_ch_ass.begin(); chit != _shape_ch_ass.end(); chit++){
		string anchor_ch = chit->first;
		vector<string> buoy_chs = chit->second;
		vector<string> anchor_bins = _shape_bin_ass.at(anchor_ch);
		cout << "sys - anchor ch " << anchor_ch << " anchor bin " << _shape_anchor_bins[anchor_ch] << endl;
		double anchorch_anchorbin_tot_yield = getTotYield(anchor_ch+_shape_anchor_bins[anchor_ch]);
		for(auto buoy_ch : buoy_chs){
			//get buoy bin for this buoy channel
			string buoy_bin = GetBuoyBin(buoy_ch);
			cout << "sys - buoy ch anchor bin " << buoy_bin << endl;
			double buoych_anchorbin_tot_yield = getTotYield(buoy_bin);
			double transfer_factor = buoych_anchorbin_tot_yield/anchorch_anchorbin_tot_yield;
			cout << "sys - anchor ch anchor bin yield " << anchorch_anchorbin_tot_yield << " buoy ch anchor bin yield " << buoych_anchorbin_tot_yield << " transfer_factor " << transfer_factor << endl;
			//tie anchor channel norm to buoy channels norm
			//buoy channel bins (b_i) have rates set to their anchor counterparts (a_i)
			//so we want to tie these bins together via a rate param initialized to the ratio N = A/B
			//where A = a_00 and B = b_00
			//such that the expectation of the rate in each buoy channel bin b_i is anchor channel bin a_i * N
			vector<string> buoy_bins = _shape_bin_ass[buoy_ch];
			cb.cp().process({proc}).bin(buoy_bins).AddSyst(cb,buoy_ch+"Norm","rateParam",SystMap<>::init(transfer_factor));
			//loop through bins in buoy_ch to set observed rates
			if(_datadriven && _asimov){ //if not asimov or not datadriven, 
				vector<string> buoy_bins = _shape_bin_ass[buoy_ch];
				for(auto buoy_bin : buoy_bins){
					if(buoy_bin.find("CR") != string::npos)
						continue;
					string bin = getBinIdx(buoy_bin);
					string anchorch_bin = anchor_ch+bin;
					double anchor_rate = GetYieldValue(anchorch_bin, proc, 1, "BuildShapeTransferFit datadriven asimov observation");
					cout << "setting obs in buoy bin " << buoy_bin << " from anchorch_bin " << anchorch_bin << " to " << transfer_factor * anchor_rate << endl;
					_obs_rates[buoy_bin] = double(int(transfer_factor * anchor_rate));
				}

			}
		}

	}
	
}


//BuildABCD - channel-to-channel ABCD
void BuildFit::BuildABCDFit(){
	cout << "Building ABCD Fit" << endl;
	//check that channel association exists within ABCD fit config
	if(_abcd_ch_ass.size() < 1){
		cout << "No channel association specified for ABCD fit. This fit config will not be written. Returning." << endl;
		return;
	}
	ch::Categories cats_abcd;
	BuildCatsSubset(_bins_superset_abcd, cats_abcd);
	//set observations based on which bins were specified in fit config for ABCD bins (shape transfer bins are set in that function)
	cb.AddProcesses(   {"*"}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, {_bkg_proc}, cats_abcd, false);
	//initialize rate of each process to 1 (bkg procs only rn) across all bins
	//such that rate * rateParam = expectation in each bin
        cb.ForEachProc([&](ch::Process *x){
		if(x->process() != _bkg_proc) return;
		//only do for bins in ABCD region
		if(find(_bins_superset_abcd.begin(), _bins_superset_abcd.end(), x->bin()) == _bins_superset_abcd.end()) return;
		cout << "abcd setting rate for bin " << x->bin() << " proc " << x->process() << endl;
            x->set_rate(1.);
        });
	//only applies ABCD treatment to background processes
	//separate ABCD factors for each bin, bins in channel tied together with extra systematics
	for(auto chit = _abcd_ch_ass.begin(); chit != _abcd_ch_ass.end(); chit++){
		string sr_ch = chit->first;
		cout << "sr_ch " << sr_ch << endl;
		vector<string> sr_bins = _abcd_bin_ass[sr_ch];
		vector<string> cr_chs = chit->second;
		for(auto sr_bin : sr_bins){
			cout << "sr_bin " << sr_bin << endl;
			vector<string> cr_bins;
			string binidx = getBinIdx(sr_bin);
			vector<string> cr_bins_matchidx; //get bins of CR channels that match this bin idx
			//set rates of cr channels to be their nominal expected yield
			for(auto cr_ch : cr_chs){
				//assuming only one bin per channel right now
				cr_bins = _abcd_bin_ass[cr_ch];
				for(auto cr_bin : cr_bins){
					if(getBinIdx(cr_bin) != binidx)
						continue;
					cr_bins_matchidx.push_back(cr_bin);
					double bkgrate_cr = GetYieldValue(cr_bin, _bkg_proc, 1, "BuildABCDFit CR rate");
					cb.cp().process({_bkg_proc}).bin({cr_bin}).AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init({cr_bin}, bkgrate_cr));
				}

			}
			for(auto bin :  cr_bins_matchidx) cout << "match bin " << bin << endl;
			//for a datadriven asimov fit (ie when the SR is blinded), set the observed yields in the SR bins to the expectation
			//set rate of bkg in sr_bin to nominally be prediction from observations in cr bins
			//A_pred = B*(C/D) from A*D = B*C
			if(_asimov && _datadriven){
				_obs_rates[sr_bin] = double(int(_obs_rates[cr_bins_matchidx[0]] * (_obs_rates[cr_bins_matchidx[1]] / _obs_rates[cr_bins_matchidx[2]])));
			}


			//since the rates were initialized to the nominal yields for these bins
			//these systmatics are initialized to 1 and can float
			//get parameter names for CR bin rate params
			string cr_rateparams = "scale_"+cr_bins_matchidx[0];
			for(int i = 1; i < (int)cr_bins_matchidx.size(); i++)
				cr_rateparams += ",scale_"+cr_bins_matchidx[i];
			//set prediction for sr bin
			//A_pred = B*(C/D) from A*D = B*C
			//tie all bins in this ABCD fit together for bkg prediction in SR bin
			cb.cp().process({_bkg_proc}).bin({sr_bin}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
						("(@0*@1/@2)",cr_rateparams)
					);
		}

	}
	
}	

//BuildABCD - MsRs ABCD within one channel
void BuildFit::BuildABCDFitSingleBin(){
	//check that channel association exists within ABCD fit config
	if(_abcd_bin_ass.size() < 1){
		cout << "No channel association specified for ABCD fit. This fit config will not be written. Returning." << endl;
		return;
	}
	//set observations based on which bins were specified in fit config	
	cb.AddProcesses(   {"*"}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, {_bkg_proc}, _cats, false);
	//initialize rate of each process to 1 (bkg procs only rn) across all bins
	//such that rate * rateParam = expectation in each bin
        cb.ForEachProc([&](ch::Process *x){
            x->set_rate(1.);
        });
	cout << "analysis " << _signalDetails[0] << " channel " << _signalDetails[1] << endl;
	//take channel 1 as the anchor channel, enforce expectation onto other channels
	//for channel connected to anchor channel, set initial value to value of non-anchor channel bin to bin in anchor channel
	//this way, the rate parameter connecting these channels is initialized to the ratio of the CR-like (signal depleted, high stats) bins across the anchor and non-anchor channel 
	cout << "# binass " << _abcd_bin_ass.size() << endl; 
	for(auto binit = _abcd_bin_ass.begin(); binit != _abcd_bin_ass.end(); binit++){
		string sr_bin = binit->first;
		vector<string> cr_bins = binit->second; //convention is that these bins go [B, C, D]
		cout << "sr bin " << sr_bin << endl;
		/*
		//set rates of non-sr (cr) bins to be their nominal expected yield
		for(auto cr_bin : cr_bins){
			double bkgrate_cr = GetYieldValue(cr_bin, _bkg_proc, 1, "BuildABCDFitSingleBin CR rate");
			ch::Process crbin_proc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],_bkg_proc,make_pair(_invcats[cr_bin],cr_bin), false, bkgrate_cr);
			cb.InsertProcess(crbin_proc);
		}
		//set rate of bkg in sr_bin to nominally be prediction from observations in cr bins
		//A_pred = B*(C/D) from A*D = B*C
		//double sr_exp = _obs_rates[cr_bins[0]] * (_obs_rates[cr_bins[1]] / _obs_rates[cr_bins[2]]);
		//ch::Process srbin_proc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],_bkg_proc,make_pair(_invcats[sr_bin],sr_bin), false, sr_exp);
		//cb.InsertProcess(srbin_proc);
		*/
		//set rate of bkg in sr_bin to nominally be prediction from observations in cr bins
		//A_pred = B*(C/D) from A*D = B*C
		if(_asimov && _datadriven){
			_obs_rates[sr_bin] = double(int(_obs_rates[cr_bins[0]] * (_obs_rates[cr_bins[1]] / _obs_rates[cr_bins[2]])));
		}

		//tie all bins in this ABCD fit together
		//since the rates were initialized to the nominal yields for these bins
		//these systmatics at initialized to 1
		//cb.cp().process({_bkg_proc}).bin(cr_bins).AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init(cr_bins, 1));
		for(auto cr_bin : cr_bins){
			double bkgrate_cr = _yields[cr_bin][_bkg_proc][1].get<double>();
			cb.cp().process({_bkg_proc}).bin({cr_bin}).AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init({cr_bin}, bkgrate_cr));
		}

		//set prediction for sr bin
		string cr_rateparams = "scale_"+cr_bins[0];
		for(int i = 1; i < (int)cr_bins.size(); i++)
			cr_rateparams += ",scale_"+cr_bins[i];
		//A_pred = B*(C/D) from A*D = B*C
		cout << "_bkg_proc " << _bkg_proc << endl;
		cb.cp().process({_bkg_proc}).bin({sr_bin}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
					("(@0*@1/@2)",cr_rateparams)
				);
		//option to pin individual rates with logNormals is set in DoSystematics()
	}

	

	//cb.PrintAll();
	//cb.PrintProcs();
	//cout << "current bins - abcd fit" << endl;
	//auto bins = cb.bin_set();
	//for(auto b : bins) cout << b << endl;
}

//DoSystematics
void BuildFit::DoSystematics(){
	for(auto syst : _systs){
		vector<string> procs = {};
		for(auto proc : syst._procs){
			if(proc == "bkg")
				procs.push_back(_bkg_proc);
			else if(proc == "sig")
				procs.push_back(_signalPoint);
		}
		if(syst._bins.size() == 0){
			for(auto bin : _bins_superset)
				syst._bins.push_back(bin);
		}
		cb.cp().process(procs).bin(syst._bins).AddSyst(cb, syst._name, syst._type, SystMap<>::init(syst._init_val));
	}
}
//WriteDatacard
void BuildFit::WriteDatacard(string datacard_dir, bool verbose){
	if(verbose)
		cout << "Writing datacard to " << datacard_dir+"/"+_signalPoint+".txt" << endl;
	cb.WriteDatacard(datacard_dir+"/"+_signalPoint+".txt");
}


void BuildFit::BuildCatsSubset(std::set<string> categories, ch::Categories& retcats){
	//build on top of last call
	retcats.clear();
	for(auto cat : categories){
		string binname = cat;
		if(_invcats.find(binname) == _invcats.end()) continue;
		int binNum = _invcats[binname];
		retcats.push_back( {binNum, binname} );
	}
}

//sets observation to sum of MC backgrounds (skips MC sig and data)
ch::Categories BuildFit::BuildCats(JSONFactory* j){
	_cats.clear(); //reset for each call
	int binNum=0;
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it) {
  		//std::cout << it.key() <<"\n";
		string binname = it.key();
		if(find(_bins_superset.begin(), _bins_superset.end(), binname) == _bins_superset.end())
			continue;
		//make sure bin is in fitconfig
		_cats.push_back( {binNum, it.key()} );
		_invcats[it.key()] = binNum;
		binNum++;
	}
	return _cats;
}
//sets observation to sum of MC backgrounds (skips MC sig and data)
std::map<std::string, float> BuildFit::BuildAsimovData(JSONFactory* j){
	_yields = j->j;
	_obs_rates.clear();
	//outer loop bin iterator
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
		//inner loop process iterator
		std::string binname = it.key();
		//cout << "binname " << binname << endl;
		//make sure bin in json exists in yaml file
		if(find(_bins_superset.begin(), _bins_superset.end(), binname) == _bins_superset.end())
			continue;
		float totalBkg = 0;
		for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
			//std::cout<< it2.key()<<"\n";
			
			if( BFTool::ContainsAnySubstring( it2.key(), sigkeys)){ //does this skip data too?
				continue;
			}
			//skip data obs
			if(it2.key() == "data") continue;
			else{
				//get the wnevents, index 1 of array
				//cout << "binname " << binname << " proc " << it2.key() << " yield " << json_array[1].get<float>() << endl;
				//std::cout<< it2.key()<<" "<<json_array[1].get<float>()<<" "<<"\n";
				totalBkg+= GetYieldValue(binname, it2.key(), 1, "BuildAsimovData");
			}
		}
		_obs_rates[binname] = float(int(totalBkg));
		if(_obs_rates[binname] == 0)
			_obs_rates[binname] = 1e-8; //avoiding fit issues
		std::cout<<"adding totalbkg: "<<binname<<" "<< float(int(totalBkg))<< " " << _obs_rates[binname] << "\n";
	}
	if(_obs_rates.size() < 1)
		cout << "Error: no observation yields set. This may be due to a mismatch in binnames between the BFI json and the fit config yaml file" << endl;
	return _obs_rates;
}
std::vector<std::string> BuildFit::GetBkgProcs(JSONFactory* j){
	_bkgprocs.clear();
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                //      std::cout<< it2.key()<<"\n";
                        if( BFTool::ContainsAnySubstring( it2.key(), sigkeys) || it2.key() == "data_obs" || BFTool::ContainsAnySubstring(it2.key(),datakeys)){
                                continue;
                        }
			//skip bkgs already there - get unique procs
			if(find(_bkgprocs.begin(), _bkgprocs.end(), it2.key()) != _bkgprocs.end())
				continue;
                        else{
				_bkgprocs.push_back(it2.key());
			}
		}
	}
	return _bkgprocs;
}
std::vector<std::string> BuildFit::GetDataProcs(JSONFactory* j){
	std::vector<std::string> bkgprocs;	
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                      //std::cout<< it2.key()<<"\n";
                        if(  BFTool::ContainsAnySubstring(it2.key(),datakeys) ){
                                bkgprocs.push_back(it2.key());
                        }
                }
        }//make this set unique
        std::set<std::string> my_bkg_set(bkgprocs.begin(), bkgprocs.end());
        std::vector<std::string> bkgprocsunique(my_bkg_set.begin(), my_bkg_set.end());
	cout << "GetDataProcs" << endl;
	for(auto c : bkgprocsunique) cout << c << endl;
	_bkgprocs = bkgprocsunique;
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

	_signalDetails = {analysis, channel, mass};
	return _signalDetails;

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
	_yields = j->j;
	_obs_rates.clear();	
	float obs_rate=0.;
	cout << "LoadDataProcesses - start" << endl;
        for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
		cout << it.key() << endl;
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
		for(int i=0; i<(int) dataKeys.size(); i++){
               		obs_rate += GetYieldValue(binname, dataKeys[i], 1, "LoadDataProcesses");
       		}
		_obs_rates[binname] = obs_rate;
		if(_obs_rates[binname] == 0)
			_obs_rates[binname] = 1e-8; //avoiding fit issues
		obs_rate=0.;
	}
	cout << "LoadDataProcesses - end" << endl;
	return _obs_rates;
}
std::map<std::string, float> BuildFit::LoadObservations(JSONFactory* j){
	_yields = j->j;
	_obs_rates.clear();
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
            	_obs_rates[binname] = GetYieldValue(binname, "data", 1, "LoadObservations");
		if(_obs_rates[binname] == 0)
			_obs_rates[binname] = 1e-8; //avoiding fit issues
        }
	return _obs_rates;	
}
double BuildFit::GetStatFracError(JSONFactory* j, std::string binName, std::vector<std::string> bkgprocs ){
	_yields = j->j;
	double fracError=0;
	double statError=0;
	double bkgYield=0;

                //assign yield to obs bin map
	for(int i=0; i<(int)bkgprocs.size(); i++){
               	bkgYield += GetYieldValue(binName, bkgprocs[i], 1, "GetStatFracError yield");
		double procStatError = GetYieldValue(binName, bkgprocs[i], 2, "GetStatFracError stat error");
		statError += procStatError*procStatError;
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
//        double bkgrateA = obs_rates[B]*(obs_rates[C]/obs_rates[D]);
	double bkgrateA = obs_rates[B]*(obs_rates[D]/obs_rates[C]);
	
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
	cb.cp().bin({B}).AddSyst(cb, "lnN_"+B, "lnN", SystMap<>::init(1.50));
	cb.cp().bin({C}).AddSyst(cb, "lnN_"+C, "lnN", SystMap<>::init(1.20));
	cb.cp().bin({D}).AddSyst(cb, "lnN_"+D, "lnN", SystMap<>::init(1.50));


	//create the A prediction bin
	// cb.cp().bin({A}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
        //  ("(@0*@1/@2)", "scale_"+B +",scale_"+C+ ",scale_"+D)
    //  );
    	cb.cp().bin({A}).AddSyst(cb, "scale_$BIN", "rateParam", SystMapFunc<>::init
          ("(@0*@2/@1)", "scale_"+B +",scale_"+C+ ",scale_"+D)
	);
	cb.PrintAll();
        cb.WriteDatacard(datacard_dir+"/"+signalPoint+"/"+signalPoint+".txt");

}
void BuildFit::BuildAsimovFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir){
	ch::Categories cats = BuildCats(j);
	//std::cout<<"building obs rates \n";
	std::map<std::string, float> obs_rates = BuildAsimovData(j);
	//std::cout<<"Getting process list\n";
	std::vector<std::string> bkgprocs = GetBkgProcs(j);
	//std::cout<<"Parse Signal point\n";
	std::vector<std::string> signalDetails = ExtractSignalDetails( signalPoint);
	//std::cout<<"Build cb objects\n";
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
        std::map<std::string, float> obs_rates = LoadDataProcesses(j, {"data_obs"});
        //std::map<std::string, float> obs_rates = LoadDataProcesses(j, {"MET18"});
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
            //std::cout<<x->bin()<<" "<<x->process()<<"\n";
            json json_array = j->j[x->bin()][x->process()];
            float temprate = json_array[1].get<float>();
            std::string c1 = "Ch1Pho1CR"; //hardcode anchor channel for now
	    std::string c2 = "Ch2Pho2CR";
            //std::string c3 = "Ch3CRLep";
            size_t foundPos = x->bin().find(c2);
	   // size_t foundPos2 = x->bin().find(c3); //very bad, need better solution to be more general
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
		cout << "set rate in bin " << x->bin() << " to " << temprate << " for process " << x->process() << endl;
	    //if( foundPos2 != std::string::npos){
            //    std::string mirrorbin = x->bin();
            //    mirrorbin.replace(foundPos2, c3.length(), c1);
            //    json_array = j->j[mirrorbin][x->process()];
            //    temprate = json_array[1].get<float>();
            //    if(temprate==0){
            //            x->set_rate(1e-8);
            //    }
            //    else{
            //            x->set_rate(temprate);
            //    }
            //}
            //else{
            //    if(temprate==0){
            //            x->set_rate(1e-8);
            //    }
            //    else{
            //            x->set_rate(temprate);
            //    }
            //}
	});
	std::vector<std::string> bincoords = { "00","10","01","11"};
        std::string ch1 = "Ch1Pho1CR";
        std::string ch2 = "Ch2Pho2CR";
	//std::vector<std::string> bincoords = { "00","10","20", "01","11","21","02","12","22"};
        //std::string ch1 = "Ch1CRHad";
        //std::string ch2 = "Ch2CRHad";
	//std::string ch3 = "Ch3CRHad";
        for(int i=0; i<(int)bincoords.size(); i++){
                cb.cp().bin({ch1+bincoords[i], ch2+bincoords[i]}).AddSyst(cb, "c1c2binShape"+bincoords[i], "lnN", SystMap<>::init(1.05));
		////cb.cp().bin({ch1+bincoords[i], ch3+bincoords[i]}).AddSyst(cb, "c1c3binShape"+bincoords[i], "lnN", SystMap<>::init(1.05));

        }
        //make ch2 normalization
        std::vector<std::string> ch2bins = channelMap["ch2"];
	std::vector<std::string> ch3bins = channelMap["ch3"];
        cb.cp().bin(ch2bins).AddSyst(cb, "c2lepNorm", "rateParam", SystMap<>::init(0.085));
	cb.cp().bin(ch3bins).AddSyst(cb, "c3lepNorm", "rateParam", SystMap<>::init(0.44));


//	cb.cp().bin(ch2bins).AddSyst(cb, "c2lepNorm", "rateParam", SystMap<>::init(0.648));
//        cb.cp().bin(ch3bins).AddSyst(cb, "c3lepNorm", "rateParam", SystMap<>::init(0.21));

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
	for(int i=0; i<(int)bincoords.size(); i++){
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
	for(int i=0; i<(int)chHad1.size(); i++){// frac error based on stat error
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
	for(int i=0; i<(int)binset.size(); i++){
	 	
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
	for( int i=0; i<(int)binset.size(); i++){		
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
