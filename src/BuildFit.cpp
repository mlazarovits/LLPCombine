#include "BuildFit.h"
#include <cmath>
#include <limits>
#include <iostream>
#include <filesystem>
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

double BuildFit::GetYieldValueOrZero(const string& bin, const string& proc, int index, const string& context) const{
	if(!_yields.is_object() || !_yields.contains(bin)){
cout << "GetYieldValueOrZero - bin " << bin << " not in yields" << endl;
		return 0.;
	}
	const json& bin_json = _yields.at(bin);
	if(!bin_json.is_object() || !bin_json.contains(proc)){
		cout << "BF JSON warning in " << context << ": missing process '" << proc
		     << "' in bin '" << bin << "', using 0" << endl;
		return 0.;
	}
	return GetYieldValue(bin, proc, index, context);
}

std::vector<std::string> BuildFit::FitBackgroundProcesses() const{
	if(_preserve_background_processes && !_datadriven){
		return _bkgprocs;
	}
	return {_bkg_proc};
}

std::vector<std::string> BuildFit::ExpandConfiguredProcesses(const std::vector<std::string>& configured) const{
	vector<string> procs = {};
	for(auto proc : configured){
		if(proc == "bkg"){
			vector<string> bkgprocs = FitBackgroundProcesses();
			procs.insert(procs.end(), bkgprocs.begin(), bkgprocs.end());
		}
		else if(proc == "sig"){
			procs.push_back(_signalPoint);
		}
		else{
			procs.push_back(proc);
		}
	}
	return procs;
}

double BuildFit::GuardRateParamInit(double value) const{
	if(!_rateparam_railguard_enabled)
		return value;
	if(_rateparam_floor_zero_init && (!std::isfinite(value) || value <= _rateparam_railguard_min))
		return _rateparam_railguard_min;
	return value;
}

std::string BuildFit::ResolveRateParamName(const std::string& name, const std::string& bin, const std::string& proc) const{
	std::string resolved = name;
	std::size_t pos = 0;
	while((pos = resolved.find("$BIN", pos)) != std::string::npos){
		resolved.replace(pos, 4, bin);
		pos += bin.size();
	}
	pos = 0;
	while((pos = resolved.find("$PROCESS", pos)) != std::string::npos){
		resolved.replace(pos, 8, proc);
		pos += proc.size();
	}
	return resolved;
}

void BuildFit::SetRateParamRange(const std::string& name, const std::vector<std::string>& bins, const std::vector<std::string>& procs){
	if(!_rateparam_railguard_enabled)
		return;
	std::set<std::string> ranged_params;
	for(auto bin : bins){
		for(auto proc : procs){
			std::string resolved = ResolveRateParamName(name, bin, proc);
			if(!ranged_params.insert(resolved).second)
				continue;
			ch::Parameter* param = cb.GetParameter(resolved);
			if(param)
				param->set_range(_rateparam_railguard_min, _rateparam_railguard_max);
		}
	}
}

void BuildFit::AddRateParam(const std::vector<std::string>& procs, const std::vector<std::string>& bins, const std::string& name, double init){
	double guarded_init = GuardRateParamInit(init);
	if(name.find("$BIN") != std::string::npos)
		cb.cp().process(procs).bin(bins).AddSyst(cb, name, "rateParam", SystMap<bin>::init(bins, guarded_init));
	else
		cb.cp().process(procs).bin(bins).AddSyst(cb, name, "rateParam", SystMap<>::init(guarded_init));
	SetRateParamRange(name, bins, procs);
}

void BuildFit::InsertDirectMCBackgroundProcesses(){
	if(_direct_mc_backgrounds_inserted)
		return;
	if(!_preserve_background_processes || _datadriven)
		return;
	cout << "Inserting direct MC background processes" << endl;
	vector<string> fit_bkgprocs = FitBackgroundProcesses();
	for(auto bin : _fit_bins){
		for(auto proc : fit_bkgprocs){
			double rate = GetYieldValueOrZero(bin, proc, 1, "InsertDirectMCBackgroundProcesses");
			cout << "direct MC bin " << bin << " proc " << proc << " rate " << rate << endl;
			ch::Process bkgproc = create_proc("*", _signalDetails[0], "13.6TeV", _signalDetails[1], proc, make_pair(_invcats[bin], bin), false, rate);
			cb.InsertProcess(bkgproc);
		}
	}
	_direct_mc_backgrounds_inserted = true;
}

std::set<string> BuildFit::ExpandToSplits(const std::set<string>& logical_bins) const{
	std::set<string> concrete;
	for(const auto& split : ActiveSplitLabels())
		for(const auto& bin : logical_bins)
			concrete.insert(DataSplit::WithSplit(bin, split));
	return concrete;
}

//fails before any model building if the BFI JSON does not provide every concrete fit bin,
//with a hint when the JSON was produced with a different split than the fit config expects
void BuildFit::ValidateInputBins(const json& yields) const{
	if(!yields.is_object())
		throw std::runtime_error("BF JSON error: top-level JSON is " + string(yields.type_name()) + ", expected object keyed by bin");
	vector<string> missing;
	for(const auto& bin : _fit_bins)
		if(!yields.contains(bin))
			missing.push_back(bin);
	if(missing.empty())
		return;
	const string& first = missing.front();
	std::ostringstream msg;
	if(_datasplit == DataSplit::DataSplitMode::Run){
		msg << "Run-split fit requested (datasplit: run) but required JSON bin '" << first << "' is missing";
		string base = DataSplit::BaseName(first);
		if(yields.contains(base))
			msg << "; the JSON has the unsplit bin '" << base << "', so it looks like BFI was run without 'split: runSplit'";
	}
	else{
		msg << "Fit config bin '" << first << "' is missing from the input JSON";
		for(const auto& label : DataSplit::KnownSplitLabels()){
			string split_bin = DataSplit::WithSplit(first, label);
			if(yields.contains(split_bin)){
				msg << "; the JSON has '" << split_bin << "', so it looks like split BFI output (set 'datasplit: run' in the fit config)";
				break;
			}
		}
	}
	msg << ". " << missing.size() << " required bin(s) missing";
	const size_t nlist = 10;
	if(missing.size() > 1){
		msg << ":";
		for(size_t i = 0; i < missing.size() && i < nlist; i++)
			msg << " " << missing[i];
		if(missing.size() > nlist)
			msg << " ...";
	}
	throw std::runtime_error(msg.str());
}

void BuildFit::ParseABCDTemplates(const YAML::Node& node){
	if(!node.IsSequence())
		throw std::runtime_error("ABCD_fit.template_processes must be a list");
	for(const auto& entry : node){
		for(const string key : {"source_channel", "target_channel", "process", "transfer_factor"})
			if(!entry[key])
				throw std::runtime_error("ABCD_fit.template_processes entry is missing required key '" + key + "'");
		ABCDTemplateProcess tmpl;
		tmpl.source_channel = entry["source_channel"].as<string>();
		tmpl.target_channel = entry["target_channel"].as<string>();
		tmpl.process = entry["process"].as<string>();
		string label = "ABCD template '" + tmpl.process + "' (" + tmpl.source_channel + " -> " + tmpl.target_channel + ")";
		string tf = entry["transfer_factor"].as<string>();
		if(tf == "auto"){
			tmpl.auto_transfer_factor = true;
		}
		else{
			size_t pos = 0;
			try{ tmpl.transfer_factor = std::stod(tf, &pos); }
			catch(const std::exception&){ pos = 0; }
			if(pos == 0 || pos != tf.size() || !std::isfinite(tmpl.transfer_factor))
				throw std::runtime_error(label + ": transfer_factor must be 'auto' or a finite number, got '" + tf + "'");
			if(tmpl.transfer_factor < 0.)
				throw std::runtime_error(label + ": transfer_factor must not be negative, got '" + tf + "'");
			tmpl.auto_transfer_factor = false;
		}
		if(tmpl.process.empty())
			throw std::runtime_error(label + ": process must not be empty");
		for(const auto& ch : {tmpl.source_channel, tmpl.target_channel})
			if(_abcd_bin_ass.find(ch) == _abcd_bin_ass.end())
				throw std::runtime_error(label + ": channel '" + ch + "' is not in ABCD_fit.bin_association");
		if(tmpl.source_channel == tmpl.target_channel)
			throw std::runtime_error(label + ": source and target channels must differ");
		if(_abcd_bin_ass.at(tmpl.target_channel).empty())
			throw std::runtime_error(label + ": target channel has no bins");
		//each target bin is templated from the source bin with the same index
		std::set<string> source_idx;
		for(const auto& bin : _abcd_bin_ass.at(tmpl.source_channel))
			source_idx.insert(getBinIdx(bin));
		for(const auto& bin : _abcd_bin_ass.at(tmpl.target_channel))
			if(!source_idx.count(getBinIdx(bin)))
				throw std::runtime_error(label + ": target bin '" + bin + "' has no source bin with index " + getBinIdx(bin));
		//the template splits the target channel into template + residual of exactly one ABCD process
		string abcd_proc = ABCDProcessOfChannel(tmpl.target_channel);
		if(abcd_proc.empty())
			throw std::runtime_error(label + ": target channel is not a control region of any ABCD_fit.channel_association process");
		if(tmpl.process == abcd_proc)
			throw std::runtime_error(label + ": process is already the ABCD process of the target channel; a template must add a distinct process");
		//the data-driven model decomposes a target channel into one template and one ABCD residual only
		for(const auto& other : _abcd_templates)
			if(other.target_channel == tmpl.target_channel)
				throw std::runtime_error("ABCD_fit.template_processes contains multiple templates for target channel '" + tmpl.target_channel
					+ "' (processes '" + other.process + "' and '" + tmpl.process + "'). Multiple templates per target channel are not supported by the current data-driven model.");
		_abcd_templates.push_back(tmpl);
	}
}

//the ABCD process whose control regions include cr_ch ("" if none); a channel shared by two ABCD processes is ambiguous
string BuildFit::ABCDProcessOfChannel(const string& cr_ch) const{
	std::set<string> procs;
	for(const auto& sr : _abcd_ch_ass)
		for(const auto& pc : sr.second)
			if(find(pc.second.begin(), pc.second.end(), cr_ch) != pc.second.end())
				procs.insert(pc.first);
	if(procs.size() > 1){
		string names;
		for(const auto& p : procs) names += (names.empty() ? "" : ", ") + p;
		throw std::runtime_error("Channel '" + cr_ch + "' is a control region of more than one ABCD process (" + names + ")");
	}
	return procs.empty() ? "" : *procs.begin();
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
	if(base["rateparam_railguard"]){
		YAML::Node guard = base["rateparam_railguard"];
		if(guard["enabled"])
			_rateparam_railguard_enabled = guard["enabled"].as<bool>();
		if(guard["min"])
			_rateparam_railguard_min = guard["min"].as<double>();
		if(guard["max"])
			_rateparam_railguard_max = guard["max"].as<double>();
		if(guard["floor_zero_init"])
			_rateparam_floor_zero_init = guard["floor_zero_init"].as<bool>();
	}
	if(_rateparam_railguard_enabled && _rateparam_railguard_max <= _rateparam_railguard_min){
		std::ostringstream msg;
		msg << "Invalid rateparam_railguard range: max (" << _rateparam_railguard_max
		    << ") must be greater than min (" << _rateparam_railguard_min << ")";
		throw std::runtime_error(msg.str());
	}
	if(_rateparam_railguard_enabled && _rateparam_railguard_min <= 0.){
		std::ostringstream msg;
		msg << "Invalid rateparam_railguard min: " << _rateparam_railguard_min
		    << " must be positive";
		throw std::runtime_error(msg.str());
	}
	if(base["shape_transfer_fit"]){
		if(base["shape_transfer_fit"]["bin_association"])
        		_shape_bin_ass = base["shape_transfer_fit"]["bin_association"].as<map<string,vector<string>>>();
		if(base["shape_transfer_fit"]["channel_association"]){
        		//_shape_ch_ass =  base["shape_transfer_fit"]["channel_association"].as<map<string,vector<string>>>();
        		auto basenode = base["shape_transfer_fit"]["channel_association"];
			for(auto it = basenode.begin(); it != basenode.end(); it++){
				string nodekey = it->first.as<string>();
				_shape_ch_ass[nodekey] = {};
				auto nodelist = it->second;
				cout << "ch " << nodekey << endl;
				if(nodelist.IsSequence()){
					cout << "node is sequence" << endl;
					_shape_ch_ass[nodekey] = nodelist.as<vector<string>>();
				}
				else if(nodelist.IsScalar()){
					cout << "node is string " << nodelist.as<string>() << endl;
					_shape_ch_ass[nodekey].push_back(nodelist.as<string>());
				}
				else if(nodelist.IsMap()){
					cout << "node is map" << endl;
					for(auto iit = nodelist.begin(); iit != nodelist.end(); iit++){
						cout << " nodemap key " << iit->first << " nodemap val " << iit->second << endl;
						_shape_ch_ass[nodekey].push_back(iit->first.as<string>());
						_shape_tf_ass[std::make_pair(nodekey,iit->first.as<string>())] = iit->second.as<double>(); 
				
					}
				}
				else{}
				

			} 
		}
	}
	if(base["ABCD_fit"]){
		//for(auto it = _abcd_bin_ass.begin(); it != _abcd_bin_ass.end(); it++){
		//	cout << "key " << it->first << " maps to " << endl;
		//	for(auto iit : it->second)
		//	       cout << " " << iit << endl;
		//}	
		if(base["ABCD_fit"]["bin_association"])
			_abcd_bin_ass = base["ABCD_fit"]["bin_association"].as<map<string,vector<string>>>();
		if(base["ABCD_fit"]["channel_association"]){
			auto basenode = base["ABCD_fit"]["channel_association"];
			for(auto it = basenode.begin(); it != basenode.end(); it++){
				string nodekey = it->first.as<string>();
				_abcd_ch_ass[nodekey] = channelmap();
				auto nodelist = it->second;
				//cout << "ch " << nodekey << endl;
				if(nodelist.IsSequence()){
					//cout << "node is sequence" << endl;
					_abcd_ch_ass[nodekey][_bkg_proc] = nodelist.as<vector<string>>();
				}
				else if(nodelist.IsMap()){
					//cout << "node is map" << endl;
					for(auto iit = nodelist.begin(); iit != nodelist.end(); iit++){
						_abcd_ch_ass[nodekey][iit->first.as<string>()] = iit->second.as<vector<string>>();
					}
				}
				else{}
			}
		
		}
		//for(auto it = _abcd_ch_ass.begin(); it != _abcd_ch_ass.end(); it++){
		//	cout << "key " << it->first << " maps to " << endl;
		//	for(auto chmap : it->second){
		//		string proc = chmap.first;
		//		cout << " proc " << proc << " with bins " << endl;
		//		for(auto b : chmap.second)
		//	       		cout << " " << b << endl;
		//	}
		//}
		if(base["ABCD_fit"]["template_processes"])
			ParseABCDTemplates(base["ABCD_fit"]["template_processes"]);
		if(base["ABCD_fit"]["ratio_denom"]){
			auto basenode = base["ABCD_fit"]["ratio_denom"];
			for(auto it = basenode.begin(); it != basenode.end(); it++){
				string nodekey = it->first.as<string>();
				string nodeval = it->second.as<string>();
				_abcd_proc_denom_ch[nodekey] = nodeval;
			}
		}
	}
	if(base["datasplit"])
		_datasplit = DataSplit::ParseDataSplitMode(base["datasplit"].as<string>());
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
	}
	for(auto it = _abcd_bin_ass.begin(); it != _abcd_bin_ass.end(); it++){
		vector<string> bins = it->second;
		for(auto bin : bins) _bins_superset.insert(bin);
		for(auto bin : bins) _bins_superset_abcd.insert(bin);
	}
	//the fit config describes the logical topology; split suffixes are added internally
	for(const auto& bin : _bins_superset){
		if(!DataSplit::SplitOf(bin).empty())
			throw std::runtime_error("Fit config bin '" + bin + "' carries a split suffix; configure unsuffixed bin names and set 'datasplit' instead");
	}
	_fit_bins = ExpandToSplits(_bins_superset);
	_fit_bins_abcd = ExpandToSplits(_bins_superset_abcd);
	ValidateSystematics();
	for(const auto& syst : _systs){
		vector<string> unknown;
		for(const auto& bin : syst._bins){
			if(!DataSplit::SplitOf(bin).empty())
				throw std::runtime_error("Systematic '" + syst._name + "' bin '" + bin + "' carries a split suffix; configure unsuffixed bins and use split_correlation");
			if(!_bins_superset.count(bin))
				unknown.push_back(bin);
		}
		if(unknown.size() > 0){
			cout << "Warning: systematic '" << syst._name << "' lists bins that are not in the fit:";
			for(const auto& bin : unknown) cout << " " << bin;
			cout << endl;
		}
	}

	if(base["asimov"].as<bool>())
		_asimov = true;
	else
		_asimov = false;
	if(base["datadriven"].as<bool>())
		_datadriven = true;
	else
		_datadriven = false;
	if(base["datasplit"]){
		if(base["datasplit"].as<string>() == "run")
			_runsplit = true;
		else if(base["datasplit"].as<string>() == "year")
			_yearsplit = true;
		else{ }	
	}
	if(base["preserve_background_processes"])
		_preserve_background_processes = base["preserve_background_processes"].as<bool>();
}

//builds categories, makes asimov obs if specified, etc
void BuildFit::PrepFit(JSONFactory* j, string signalPoint, vector<string> datakeys){
	ValidateInputBins(j->j);
	if(_datasplit != DataSplit::DataSplitMode::None){
		cout << "Building " << DataSplit::ModeName(_datasplit) << "-split fit with splits:";
		for(const auto& split : ActiveSplitLabels()) cout << " " << split;
		cout << endl;
	}
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
	if(!_preserve_background_processes || _datadriven)
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
		//sum over all processes (set from asimov)
		double obs = SumObs(x->bin());
           	x->set_rate(obs);
	    //cout << x->bin() << " " << x->process() << " " << x->rate() << endl;
        });

}

void BuildFit::SetSignalRates(){
        cb.AddProcesses(   {_signalDetails[2]}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, {_signalPoint}, _cats, true);
	//set signal rates
	cb.ForEachProc([&](ch::Process *x){
		if(x->process() == _signalPoint){
			double yield = _yields[x->bin()][x->process()][1].get<double>();
			if(yield == 0)
				yield = 1e-8;
			x->set_rate(yield);
		}
	});

}

//create 1 bkg process that will behave like the bkg process from data in the datacard
void BuildFit::sumBkgs(){
	string newproc = "bkg";
	for(auto bin : _fit_bins){
		double wt_yield = 0;
		double unwt_yield = 0;
		double sq_err = 0;
		for(auto proc : _bkgprocs){
			wt_yield += _yields[bin][proc][1].get<double>();
			unwt_yield += _yields[bin][proc][0].get<double>();
			double err = _yields[bin][proc][2].get<double>();
			sq_err += err*err;
		}
		//add to json array
		_yields[bin][newproc] = {unwt_yield, wt_yield, sqrt(sq_err)}; 
	}
}

void BuildFit::BuildShapeTransferFit(){
	vector<string> splits;
	if(_yearsplit){
		splits = {"16","17","18","22","23","24","25"};
	}
	else if(_runsplit){
		splits = {"Run2","Run3"};

	}
	else
		splits = {""};	
	for(auto split : splits)
		BuildShapeTransferFit(split);
}


//channel name convention goes
//Ch#IDBin where Bin = xy coord in MsRs plane
void BuildFit::BuildShapeTransferFit(const string& split){
	cout << "Building Shape Transfer Fit" << endl;
	if(_preserve_background_processes && !_datadriven){
		InsertDirectMCBackgroundProcesses();
		cout << "Skipping shape-transfer background constraints for preserved MC background mode" << endl;
		return;
	}
	if(_shape_ch_ass.size() < 1){
		cout << "No channel association specified for shape transfer fit. This fit config will not be written. Returning." << endl;
		return;
	}
	for(const auto& split : ActiveSplitLabels())
		BuildShapeTransferFitForSplit(split);
}

//shape-transfer model for one split: every yield read and every bin constrained carries this split's suffix,
//so transfer factors are never formed across splits
void BuildFit::BuildShapeTransferFitForSplit(const string& split){
	using DataSplit::WithSplit;
	//take channel 1 as the anchor channel, enforce expectation onto other channels
	//for channel connected to anchor channel, set initial value to value of non-anchor channel bin to bin in anchor channel
	//this way, the rate parameter connecting these channels is initialized to the ratio of the CR-like (signal depleted, high stats) bins across the anchor and non-anchor channel
	vector<string> fit_bkgprocs = FitBackgroundProcesses();
	for(auto chit = _shape_ch_ass.begin(); chit != _shape_ch_ass.end(); chit++){
		//get anchor channel as specified from fit config
		string anchor_ch = chit->first;
//cout << "anchor_ch " << anchor_ch << endl;
		vector<string> buoy_chs = chit->second;
		vector<string> anchor_bins = _shape_bin_ass[anchor_ch];
		//set yields in every bin of the anchor channel to their nominal value
		for(int b = 0; b < (int)anchor_bins.size(); b++){
			string anchor_bin = WithSplit(anchor_bins[b], split);
			string bin = getBinIdx(anchor_bins[b]);
			for(auto proc : fit_bkgprocs){
				double anchor_rate = GetYieldValueOrZero(anchor_bin, proc, 1, "BuildShapeTransferFit anchor rate");
//cout << "proc " << proc << " bin " << bin << " anchor " << anchor_bin << " rate " << anchor_rate << endl;
				if(anchor_rate == 0) anchor_rate = 1e-8;
				//set yield
				ch::Process anchorproc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],proc,make_pair(_invcats[anchor_bin],anchor_bin), false, anchor_rate);
				cb.InsertProcess(anchorproc);
				//set yield in buoy channels to that of connected anchor channel for this process
				for(int i = 0; i < (int)buoy_chs.size(); i++){
					vector<string> buoy_bins = _shape_bin_ass[buoy_chs[i]];
					//make sure bin configuration for anchor channel matches that for buoy channel
					if(buoy_bins.size() != anchor_bins.size()){
						cout << "Buoy channel " << buoy_chs[i] << " has " << buoy_bins.size() << " bins which does not map to anchor channel " << anchor_ch << " which has " << anchor_bins.size() << ". Skipping." << endl;
						continue;
					}
					string buoy_bin;
					if(bin == _shape_anchor_bins[buoy_chs[i]])
						buoy_bin = GetBuoyBin(buoy_chs[i]);
					else
						buoy_bin = buoy_chs[i]+bin;
					//make sure anchor channel bin exists in buoy channel
					if(find(buoy_bins.begin(), buoy_bins.end(), buoy_bin) == buoy_bins.end())
						continue;
					buoy_bin = WithSplit(buoy_bin, split);
					//match bin indices (assuming that bins have been defined identically)
					pair<string, string> ch_pair = std::make_pair(anchor_ch, buoy_chs[i]);
					ch::Process buoyproc;
					if(_shape_tf_ass.find(ch_pair) != _shape_tf_ass.end()){ //if transfer factor is specified, set the expected buoy rate to TF * anchor so constraints can be applied later
						buoyproc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],proc,make_pair(_invcats[buoy_bin],buoy_bin), false, anchor_rate*_shape_tf_ass[ch_pair]);
					}	
					else
						buoyproc = create_proc("*",_signalDetails[0],"13.6TeV",_signalDetails[1],proc,make_pair(_invcats[buoy_bin],buoy_bin), false, anchor_rate);
					cb.InsertProcess(buoyproc);
				}
			}
		}
	}

	//add systematics to tie bins and channels together
	for(auto chit = _shape_ch_ass.begin(); chit != _shape_ch_ass.end(); chit++){
		string anchor_ch = chit->first;
		vector<string> buoy_chs = chit->second;
		string anchorch_anchorbin = WithSplit(anchor_ch+_shape_anchor_bins[anchor_ch], split);
		double anchorch_anchorbin_tot_yield = getTotYield(anchorch_anchorbin);
		for(auto buoy_ch : buoy_chs){
			//get buoy bin for this buoy channel
			string buoy_bin = WithSplit(GetBuoyBin(buoy_ch), split);
			double buoych_anchorbin_tot_yield = getTotYield(buoy_bin);
			//the transfer factor must be well defined here; the rateParam railguard is not meant to hide a 0/0 or x/0
			double transfer_factor = anchorch_anchorbin_tot_yield > 0. ? buoych_anchorbin_tot_yield/anchorch_anchorbin_tot_yield : std::numeric_limits<double>::quiet_NaN();
			//set transfer factor if specified
			if(_shape_tf_ass.find(std::make_pair(anchor_ch, buoy_ch)) != _shape_tf_ass.end())
				transfer_factor = _shape_tf_ass[std::make_pair(anchor_ch, buoy_ch)];
			if(((anchorch_anchorbin_tot_yield <= 0.) && _shape_anchor_bins[anchor_ch] != "") || !std::isfinite(transfer_factor) || transfer_factor < 0.){
				std::ostringstream msg;
				msg << "Cannot construct shape transfer for " << buoy_ch << (split.empty() ? "" : ", " + split)
				    << " (anchor channel " << anchor_ch << "): anchor bin " << anchorch_anchorbin << " yield " << anchorch_anchorbin_tot_yield
				    << ", buoy bin " << buoy_bin << " yield " << buoych_anchorbin_tot_yield
				    << "; the transfer factor buoy/anchor must be finite and non-negative, which needs a positive anchor yield";
				throw std::runtime_error(msg.str());
			}
			//tie anchor channel norm to buoy channels norm
			//buoy channel bins (b_i) have rates set to their anchor counterparts (a_i)
			//so we want to tie these bins together via a rate param initialized to the ratio N = A/B
			//where A = a_00 and B = b_00
			//such that the expectation of the rate in each buoy channel bin b_i is anchor channel bin a_i * N
			vector<string> buoy_bins;
			for(const auto& logical_bin : _shape_bin_ass[buoy_ch])
				buoy_bins.push_back(WithSplit(logical_bin, split));
			if(_preserve_background_processes && !_datadriven){
				for(auto proc : fit_bkgprocs){
					double anchor_yield = GetYieldValueOrZero(anchorch_anchorbin, proc, 1, "BuildShapeTransferFit proc transfer anchor");
					double buoy_yield = GetYieldValueOrZero(buoy_bin, proc, 1, "BuildShapeTransferFit proc transfer buoy");
					double proc_transfer_factor = anchor_yield > 0 ? buoy_yield/anchor_yield : 1.;
					if(_shape_tf_ass.find(std::make_pair(anchor_ch, buoy_ch)) == _shape_tf_ass.end()){
						//TF already incorporated in expected rate for externally set TF, dont need rate param
						//proc_transfer_factor = _shape_tf_ass[std::make_pair(anchor_ch, buoy_ch)];
						AddRateParam({proc}, buoy_bins, WithSplit(buoy_ch+"Norm_"+proc, split), proc_transfer_factor);
					}
				}
			}
			else{
				//TF already incorporated in expected rate for externally set TF, dont need rate param
				if(_shape_tf_ass.find(std::make_pair(anchor_ch, buoy_ch)) == _shape_tf_ass.end())
					AddRateParam({_bkg_proc}, buoy_bins, WithSplit(buoy_ch+"Norm", split), transfer_factor);
			}
			//loop through bins in buoy_ch to set observed rates
			if(_datadriven && _asimov){ //if not asimov or not datadriven,
				for(const auto& logical_bin : _shape_bin_ass[buoy_ch]){
					if(logical_bin.find("CR") != string::npos)
						continue;
					string sr_bin = WithSplit(logical_bin, split);
					string anchorch_bin = WithSplit(anchor_ch+getBinIdx(logical_bin), split);
					double anchor_rate = GetYieldValue(anchorch_bin, _bkg_proc, 1, "BuildShapeTransferFit datadriven asimov observation");
					_obs_rates[sr_bin][_bkg_proc] = double(int(transfer_factor * anchor_rate));
//cout << "SHAPE asimov yields for srbin " << sr_bin << " proc " << _bkg_proc << ": " << transfer_factor * anchor_rate << " anchor_rate " << anchor_rate << " tf " << transfer_factor << " _obs_rates " << _obs_rates[sr_bin][_bkg_proc] << endl;
				}

			}
		}

	}

}

//BuildABCD - channel-to-channel ABCD
//
//construction order (per split, never mixing splits):
//	1. decompose every template target bin into template + ABCD residual (ComputeABCDTemplates)
//	2. add the ABCD processes
//	3. create each CR rateParam once, from the residual where the channel is templated, else from the data yield
//	4. create the SR formulas A = B*C/D and, for a datadriven asimov fit, set the SR observation to the sum of the predictions
//	5. add the template processes and their own transfer-factor rateParams
void BuildFit::BuildABCDFit(){
	cout << "Building ABCD Fit" << endl;
	if(_preserve_background_processes && !_datadriven){
		InsertDirectMCBackgroundProcesses();
		cout << "Skipping ABCD background constraints for preserved MC background mode" << endl;
		return;
	}
	//check that channel association exists within ABCD fit config
	if(_abcd_ch_ass.size() < 1){
		cout << "No channel association specified for ABCD fit. This fit config will not be written. Returning." << endl;
		return;
	}
	//all template yields are known (and validated) before any ABCD rateParam is created
	ComputeABCDTemplates();
	ch::Categories cats_abcd;
	BuildCatsSubset(_fit_bins_abcd, cats_abcd);
	//for having separate processes in different bins
	for(auto c : cats_abcd){
		//only do for specified split
		if(c.second.find("SR") == string::npos){
			string bkgproc = getProcess(c.second);
			//only set rate for given process
			//initialize rate of each process to 1 (bkg procs only rn) across all bins
			//such that rate * rateParam = expectation in each bin
			ch::Process cbproc = create_proc("*", _signalDetails[0], "13.6TeV", _signalDetails[1], bkgproc, c, false, 1.);
			cb.InsertProcess(cbproc);
		}
		//assume only SRs are getting predicted from ABCD
		else{
			auto chit = _abcd_ch_ass.find(getChannel(c.second));
			if(chit != _abcd_ch_ass.end())
				for(auto it = chit->second.begin(); it != chit->second.end(); it++){
					//only set rate for given process
					//initialize rate of each process to 1 (bkg procs only rn) across all bins
					//such that rate * rateParam = expectation in each bin
					ch::Process cbproc = create_proc("*", _signalDetails[0], "13.6TeV", _signalDetails[1], it->first, c, false, 1.);
					cb.InsertProcess(cbproc);
				}
		}
	}
	for(const auto& split : ActiveSplitLabels()){
		BuildABCDConstraintsForSplit(split);
	}
	for(const auto& result : _abcd_template_results)
		AddABCDTemplateProcess(result);
}

//data-driven yield of a bin: the summed "bkg" process, i.e. the data in a datadriven fit
double BuildFit::DataDrivenYield(const string& bin, const string& context) const{
	return GetYieldValue(bin, _bkg_proc, 1, context);
}

//yield that initializes the ABCD rateParam of a control-region bin: the residual after the
//template contribution for a templated target bin, the full data-driven yield otherwise
double BuildFit::ABCDControlYield(const string& crbin) const{
	auto it = _abcd_template_bins.find(crbin);
	if(it != _abcd_template_bins.end())
		return it->second.residual_yield;
	return DataDrivenYield(crbin, "BuildABCDFit CR rate");
}

//decomposes each target-channel bin of one split as
//	target = template + residual,  template = TF * source (same bin index, same split)
//TF auto: min over bins with a non-zero source yield of target/source, so no residual is negative;
//a numeric TF that makes any residual negative is a configuration error
ABCDTemplateSplitResult BuildFit::ComputeABCDTemplate(const ABCDTemplateProcess& tmpl, const string& split) const{
	using DataSplit::WithSplit;
	string label = "ABCD template '" + tmpl.process + "' (" + tmpl.source_channel + " -> " + tmpl.target_channel + (split.empty() ? "" : ", " + split) + ")";
	ABCDTemplateSplitResult result;
	result.config = tmpl;
	result.split = split;
	string abcd_proc = ABCDProcessOfChannel(tmpl.target_channel);
	auto tbins = _abcd_bin_ass.find(tmpl.target_channel);
	if(tbins == _abcd_bin_ass.end() || tbins->second.empty())
		throw std::runtime_error(label + ": target channel has no bins");
	for(const auto& logical_bin : tbins->second){
		ABCDTemplateBinResult b;
		b.target_bin = WithSplit(logical_bin, split);
		b.source_bin = WithSplit(tmpl.source_channel + getBinIdx(logical_bin), split);
		b.template_process = tmpl.process;
		b.abcd_process = abcd_proc;
		b.source_yield = DataDrivenYield(b.source_bin, "ABCD template source yield");
		b.target_yield = DataDrivenYield(b.target_bin, "ABCD template target yield");
		result.bins.push_back(b);
	}
	double tf = tmpl.transfer_factor;
	if(tmpl.auto_transfer_factor){
		//bins with no source yield do not constrain the ratio
		tf = std::numeric_limits<double>::infinity();
		for(const auto& b : result.bins){
			if(b.source_yield > 0.){
				cout << " bin " << b.target_bin << " tf " << b.target_yield / b.source_yield << endl;
				tf = std::min(tf, b.target_yield / b.source_yield);
			}
		}
		if(!std::isfinite(tf))
			throw std::runtime_error(label + ": cannot compute an automatic transfer factor, every source bin has zero yield");
	}
	cout << "Transfer factor for template from " << tmpl.source_channel << " to " << tmpl.target_channel << ": " << tf << endl;
	result.transfer_factor = tf;
	for(auto& b : result.bins){
		b.transfer_factor = tf;
		b.template_yield = tf * b.source_yield;
		b.residual_yield = b.target_yield - b.template_yield;
		//tolerate only floating-point rounding (auto TF makes the minimizing bin's residual exactly zero)
		double tol = 1e-9 * std::max(1., std::fabs(b.target_yield));
		if(b.residual_yield < -tol){
			std::ostringstream msg;
			msg << "ABCD template '" << tmpl.process << "' gives a negative residual\n"
			    << "  split: " << (split.empty() ? "none" : split) << "\n"
			    << "  bin index: " << getBinIdx(b.target_bin) << "\n"
			    << "  source bin: " << b.source_bin << "\n"
			    << "  target bin: " << b.target_bin << "\n"
			    << "  source yield: " << b.source_yield << "\n"
			    << "  target yield: " << b.target_yield << "\n"
			    << "  transfer factor: " << tf << (tmpl.auto_transfer_factor ? " (auto)" : "") << "\n"
			    << "  template prediction: " << b.template_yield << "\n"
			    << "  residual: " << b.residual_yield;
			throw std::runtime_error(msg.str());
		}
		if(b.residual_yield < 0.){
			b.residual_yield = 0.;
			b.template_yield = b.target_yield;
		}
		//invariant: the two data-driven components add back up to the observed target yield
		if(std::fabs(b.template_yield + b.residual_yield - b.target_yield) > tol)
			throw std::logic_error(label + ": template + residual does not reproduce the target yield in " + b.target_bin);
	}
	return result;
}

void BuildFit::ComputeABCDTemplates(){
	_abcd_template_results.clear();
	_abcd_template_bins.clear();
	for(const auto& tmpl : _abcd_templates){
		for(const auto& split : ActiveSplitLabels()){
			ABCDTemplateSplitResult result = ComputeABCDTemplate(tmpl, split);
			cout << "ABCD template " << tmpl.process << (split.empty() ? "" : " " + split) << " transfer factor = " << result.transfer_factor
			     << (tmpl.auto_transfer_factor ? " (auto)" : "") << endl;
			for(const auto& b : result.bins)
				if(!_abcd_template_bins.emplace(b.target_bin, b).second)
					throw std::logic_error("ABCD target bin '" + b.target_bin + "' is templated more than once");
			_abcd_template_results.push_back(result);
		}
	}
}

//ABCD relations for one split: A_split = B_split * C_split / D_split, so an SR prediction
//only ever references control-region rateParams of its own split
void BuildFit::BuildABCDConstraintsForSplit(const string& split){
	using DataSplit::WithSplit;
	//each (CR bin, process) rateParam is created exactly once, even if several SR bins or channels use it
	std::set<pair<string, string>> cr_rateparams_done;
	//SR bin -> process -> datadriven asimov prediction
	//map<string, map<string, double>> sr_predictions;
	//only applies ABCD treatment to background processes
	//separate ABCD factors for each bin, bins in channel tied together with extra systematics
	for(auto chit = _abcd_ch_ass.begin(); chit != _abcd_ch_ass.end(); chit++){
		string sr_ch = chit->first;
		vector<string> sr_bins = _abcd_bin_ass[sr_ch];
                for(auto pit = chit->second.begin(); pit != chit->second.end(); pit++){
			//for each process get each bin associated with sr ch
			string proc = pit->first;
			vector<string> cr_chs = pit->second;
			for(auto logical_sr_bin : sr_bins){
				string sr_bin = WithSplit(logical_sr_bin, split);
				string binidx = getBinIdx(logical_sr_bin);
				//set rate of bkg in sr_bin to nominally be prediction from observations in cr bins
				//A_pred = B*(C/D) from A*D = B*C
				vector<string> cr_bins_matchidx;
				for(auto cr_ch : cr_chs)
					for(auto cr_bin : _abcd_bin_ass[cr_ch])
						if(getBinIdx(cr_bin) == binidx)
							cr_bins_matchidx.push_back(WithSplit(cr_bin, split));
				if(cr_bins_matchidx.size() < 3)
					throw std::runtime_error("ABCD SR bin '" + sr_bin + "' (process '" + proc + "') needs three control-region bins with index " + binidx + ", found " + std::to_string(cr_bins_matchidx.size()));
				//CR yields for the ABCD process: residual yields in templated target bins
				vector<double> cr_yields;
				for(const auto& crbin : cr_bins_matchidx){
					cr_yields.push_back(ABCDControlYield(crbin));
					if(cr_rateparams_done.insert({crbin, proc}).second)
						AddRateParam({proc}, {crbin}, "scale_$BIN", cr_yields.back());
				}
				//for a datadriven asimov fit (ie when the SR is blinded), the observed SR yield is the expectation
				if(_asimov && _datadriven){
					double pred = cr_yields[0] * (cr_yields[1] / cr_yields[2]);
//cout << "ABCD asimov prediction for SR bin '" << sr_bin << "' (process '" << proc << "') is " << pred << endl;
					if(!std::isfinite(pred)){
						std::ostringstream msg;
						msg << "ABCD asimov prediction for SR bin '" << sr_bin << "' (process '" << proc << "') is not finite: "
						    << cr_bins_matchidx[0] << " = " << cr_yields[0] << ", " << cr_bins_matchidx[1] << " = " << cr_yields[1]
						    << ", " << cr_bins_matchidx[2] << " = " << cr_yields[2];
						throw std::runtime_error(msg.str());
					}
					//sr_predictions[sr_bin][proc] = double(int(pred));
					_obs_rates[sr_bin][proc] = double(int(pred));
//cout << "ABCD asimov yields for srbin " << sr_bin << " proc " << proc << " _obs_rates " << _obs_rates[sr_bin][proc] << endl;
				}
				string cr_rateparams = "scale_"+cr_bins_matchidx[0];
				for(int i = 1; i < (int)cr_bins_matchidx.size(); i++)
					cr_rateparams += ",scale_"+cr_bins_matchidx[i];
				cb.cp().process({proc}).bin({sr_bin}).AddSyst(cb, "scale_$BIN_$PROCESS", "rateParam", SystMapFunc<>::init
								("(@0*@1/@2)",cr_rateparams)
					);
			}
		}

	}
	//the blinded SR observation is exactly the sum of its background predictions (never the loaded SR data)
	//overwrites all entries in _obs_rates[bin], even for other procs
	//for(const auto& sp : sr_predictions){
	//	//_obs_rates[sp.first] = sp.second;
	//}
}

//adds the template process to every target-channel bin of its split with
//	rate = source-channel yield (same bin index, same split) * scale_<target>_<process>[_<split>]
//the ABCD residual of these bins was already used for their CR rateParams; for a datadriven asimov fit
//the target observation is written as the explicit template + residual decomposition
void BuildFit::AddABCDTemplateProcess(const ABCDTemplateSplitResult& result){
	using DataSplit::WithSplit;
	const ABCDTemplateProcess& tmpl = result.config;
	std::set<string> target_bins;
	map<string, double> source_rates;
	for(const auto& b : result.bins){
		target_bins.insert(b.target_bin);
		source_rates[b.target_bin] = b.source_yield;
	}
	ch::Categories cats;
	BuildCatsSubset(target_bins, cats);
	if(cats.size() != target_bins.size())
		throw std::runtime_error("ABCD template '" + tmpl.process + "' (" + tmpl.target_channel + (result.split.empty() ? "" : ", " + result.split) + "): target bins missing from the fit categories");
	cb.AddProcesses(   {"*"}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, {tmpl.process}, cats, false);
        cb.ForEachProc([&](ch::Process *x){
		if(x->process() != tmpl.process) return;
		auto it = source_rates.find(x->bin());
		if(it != source_rates.end())
			x->set_rate(it->second);
        });
	vector<string> systbins(target_bins.begin(), target_bins.end());
	AddRateParam({tmpl.process}, systbins, WithSplit("scale_"+tmpl.target_channel+"_$PROCESS", result.split), result.transfer_factor);
	if(!(_asimov && _datadriven))
		return;
	for(const auto& b : result.bins){
		double original = SumObs(b.target_bin);
		auto& obs = _obs_rates[b.target_bin];
		obs.clear();
		obs[b.abcd_process] = b.residual_yield;
		obs[b.template_process] = b.template_yield;
		//an empty target bin keeps the loaded observation (floored away from zero)
		if(b.residual_yield <= 0. && b.template_yield <= 0.)
			obs[b.abcd_process] = original;
		if(std::fabs(SumObs(b.target_bin) - original) > 1e-6 * std::max(1., std::fabs(original)))
			throw std::logic_error("ABCD template decomposition changed the observation of '" + b.target_bin + "'");
	}
}


//BuildABCD - MsRs ABCD within one channel
void BuildFit::BuildABCDFitSingleBin(){
	//bin_association keys are SR bins here, not channels, so there is no split-aware expansion of them
	if(_datasplit != DataSplit::DataSplitMode::None)
		throw std::runtime_error("BuildABCDFitSingleBin does not support datasplit '" + DataSplit::ModeName(_datasplit) + "'");
	//check that channel association exists within ABCD fit config
	if(_preserve_background_processes && !_datadriven){
		InsertDirectMCBackgroundProcesses();
		cout << "Skipping single-bin ABCD background constraints for preserved MC background mode" << endl;
		return;
	}
	if(_abcd_bin_ass.size() < 1){
		cout << "No channel association specified for ABCD fit. This fit config will not be written. Returning." << endl;
		return;
	}
	//set observations based on which bins were specified in fit config	
	vector<string> fit_bkgprocs = FitBackgroundProcesses();
	cb.AddProcesses(   {"*"}, {_signalDetails[0]}, {"13.6TeV"}, {_signalDetails[1]}, fit_bkgprocs, _cats, false);
	//initialize rate of each process to 1 (bkg procs only rn) across all bins
	//such that rate * rateParam = expectation in each bin
        cb.ForEachProc([&](ch::Process *x){
	    if(find(fit_bkgprocs.begin(), fit_bkgprocs.end(), x->process()) == fit_bkgprocs.end()) return;
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
			double bkgrate_cr = _yields[cr_bin][_bkg_proc][1].get<double>();
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
			_obs_rates[sr_bin][_bkg_proc] = double(int(_obs_rates[cr_bins[0]][_bkg_proc] * (_obs_rates[cr_bins[1]][_bkg_proc] / _obs_rates[cr_bins[2]][_bkg_proc])));
		}

		//tie all bins in this ABCD fit together
		//since the rates were initialized to the nominal yields for these bins
		//these systmatics at initialized to 1
		//cb.cp().process({_bkg_proc}).bin(cr_bins).AddSyst(cb, "scale_$BIN", "rateParam", SystMap<bin>::init(cr_bins, 1));
		for(auto cr_bin : cr_bins){
			for(auto proc : fit_bkgprocs){
				double bkgrate_cr = GetYieldValueOrZero(cr_bin, proc, 1, "BuildABCDFitSingleBin CR rate");
				string pname = (_preserve_background_processes && !_datadriven) ? "scale_"+cr_bin+"_"+proc : "scale_$BIN";
				AddRateParam({proc}, {cr_bin}, pname, bkgrate_cr);
			}
		}

		//set prediction for sr bin
		for(auto proc : fit_bkgprocs){
			string cr_rateparams;
			if(_preserve_background_processes && !_datadriven){
				cr_rateparams = "scale_"+cr_bins[0]+"_"+proc;
				for(int i = 1; i < (int)cr_bins.size(); i++)
					cr_rateparams += ",scale_"+cr_bins[i]+"_"+proc;
			}
			else{
				cr_rateparams = "scale_"+cr_bins[0];
				for(int i = 1; i < (int)cr_bins.size(); i++)
					cr_rateparams += ",scale_"+cr_bins[i];
			}
			//A_pred = B*(C/D) from A*D = B*C
			cout << "_bkg_proc " << proc << endl;
			string sr_param = (_preserve_background_processes && !_datadriven) ? "scale_"+sr_bin+"_"+proc : "scale_$BIN";
			cb.cp().process({proc}).bin({sr_bin}).AddSyst(cb, sr_param, "rateParam", SystMapFunc<>::init
						("(@0*@1/@2)",cr_rateparams)
					);
		}
		//option to pin individual rates with logNormals is set in DoSystematics()
	}

	

	//cb.PrintAll();
	//cb.PrintProcs();
	//cout << "current bins - abcd fit" << endl;
	//auto bins = cb.bin_set();
	//for(auto b : bins) cout << b << endl;
}

//a $BIN nuisance resolves to a different name in every split category (the bin names carry _Run2/_Run3),
//so it can never be one shared nuisance; asking for that is a configuration error, not something to ignore
void BuildFit::ValidateSystematics() const{
	if(_datasplit == DataSplit::DataSplitMode::None)
		return;
	for(const auto& syst : _systs)
		if(syst._split_correlation == SplitCorrelation::Correlated && syst._name.find("$BIN") != string::npos)
			throw std::runtime_error("Systematic '" + syst._name + "' has split_correlation: correlated but its name contains $BIN. "
				"With datasplit: " + DataSplit::ModeName(_datasplit) + " the bin names carry a split suffix, so $BIN resolves to a "
				"different nuisance in each split and cannot be correlated. Remove $BIN from the name or use split_correlation: uncorrelated.");
}

//nuisance name of a configured systematic in one split; correlated systematics (and every unsplit fit)
//keep the configured name, names built from $BIN already differ per split through the bin name
string BuildFit::SystematicNameForSplit(const yamlSys& syst, const string& split) const{
	if(split.empty() || syst._split_correlation == SplitCorrelation::Correlated || syst._name.find("$BIN") != string::npos)
		return syst._name;
	return DataSplit::WithSplit(syst._name, split);
}

//DoSystematics
void BuildFit::DoSystematics(){
	vector<string> splits;
	if(_yearsplit){
		splits = {"16","17","18","22","23","24","25"};
	}
	else if(_runsplit){
		splits = {"Run2","Run3"};

	}
	else
		splits = {""};	
	for(auto split : splits)
		DoSystematics(split);

}
//DoSystematics
void BuildFit::DoSystematics(const string& split){
	cout << "Adding additional systematics" << endl;
	for(auto syst : _systs){
		vector<string> procs = ExpandConfiguredProcesses(syst._procs);
		vector<string> logical_bins = syst._bins;
		if(logical_bins.size() == 0)
			logical_bins.assign(_bins_superset.begin(), _bins_superset.end());
		//nuisance name -> concrete bins it applies to
		map<string, vector<string>> nuisance_bins;
		for(const auto& split : ActiveSplitLabels())
			for(const auto& bin : logical_bins)
				nuisance_bins[SystematicNameForSplit(syst, split)].push_back(DataSplit::WithSplit(bin, split));
		for(const auto& nb : nuisance_bins){
				cb.cp().process(procs).bin(nb.second).AddSyst(cb, nb.first, syst._type, SystMap<>::init(syst._init_val));
		}
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
		if(_fit_bins.count(binname) == 0)
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
	_obs_rates.clear();
	map<string, float> obsrates;
	//outer loop bin iterator
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
		//inner loop process iterator
		std::string binname = it.key();
		//cout << "binname " << binname << endl;
		//make sure bin in json exists in yaml file
		if(_fit_bins.count(binname) == 0)
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
				json json_array = it2.value();
				//cout << "binname " << binname << " proc " << it2.key() << " yield " << json_array[1].get<float>() << endl;
				//std::cout<< it2.key()<<" "<<json_array[1].get<float>()<<" "<<"\n";
				totalBkg+= GetYieldValue(binname, it2.key(), 1, "BuildAsimovData");
				_obs_rates[binname][it2.key()] = float(int(GetYieldValue(binname, it2.key(), 1, "BuildAsimovData")));
				if(_obs_rates[binname][it2.key()] == 0){
					_obs_rates[binname][it2.key()] = 1e-8; //avoiding fit issues
				}
			}
		}
		obsrates[binname] = float(int(totalBkg));
		if(obsrates[binname] == 0){
			obsrates[binname] = 1e-8; //avoiding fit issues
		}
		std::cout<<"adding totalbkg: "<<binname<<" "<< float(int(totalBkg))<< " " << obsrates[binname] << "\n";
	}
	if(obsrates.size() < 1)
		cout << "Error: no observation yields set. This may be due to a mismatch in binnames between the BFI json and the fit config yaml file" << endl;
	return obsrates;
}
std::vector<std::string> BuildFit::GetBkgProcs(JSONFactory* j){
	_bkgprocs.clear();
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                //      std::cout<< it2.key()<<"\n";
                        if( BFTool::ContainsAnySubstring( it2.key(), sigkeys) || it2.key() == "data_obs" || IsDataKey(it2.key()) || IsDataEraKey(it2.key())){
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
                        if( IsDataKey(it2.key()) ){
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
	_obs_rates.clear();	
	float obs_rate=0.;
	map<string, float> obs_rates;
	cout << "LoadDataProcesses - start" << endl;
        for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
		cout << it.key() << endl;
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
                obs_rates[binname] = 0;
		for(int i=0; i<(int) dataKeys.size(); i++){
			obs_rate += GetYieldValue(binname, dataKeys[i], 1, "LoadDataProcesses");
			_obs_rates[binname][dataKeys[i]] = GetYieldValue(binname, dataKeys[i], 1, "LoadDataProcesses");
			if(_obs_rates[binname][dataKeys[i]]== 0)
				_obs_rates[binname][dataKeys[i]] = 1e-8; //avoiding fit issues
       		}
		obs_rates[binname] = obs_rate;
		obs_rate = 0.;
	}
	cout << "LoadDataProcesses - end" << endl;
	return obs_rates;
}
std::map<std::string, float> BuildFit::LoadObservations(JSONFactory* j){
	_obs_rates.clear();
	map<string, float> obs_rates;
	for (json::iterator it = j->j.begin(); it != j->j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                //assign yield to obs bin map
		//need to set _obs_rates for given process if it contributes to a certain CR
		string bkgproc = getProcess(binname);
		_obs_rates[binname][bkgproc] = GetYieldValue(binname, "data", 1, "LoadObservations");
		obs_rates[binname] = GetYieldValue(binname, "data", 1, "LoadObservations");
		if(_obs_rates[binname][bkgproc] == 0)
			_obs_rates[binname][bkgproc] = 1e-8; //avoiding fit issues
		if(obs_rates[binname] == 0)
			obs_rates[binname] = 1e-8; //avoiding fit issues
        }
	return obs_rates;	
}
double BuildFit::GetStatFracError(JSONFactory* j, std::string binName, std::vector<std::string> bkgprocs ){
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

