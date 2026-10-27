#ifndef BUILDFIT_H
#define BUILDFIT_H

#include "JSONFactory.h"
#include "BuildFitTools.h"
#include "DataSplit.h"
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <yaml-cpp/yaml.h>
#include <utility>

#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"
//using ch::syst::SystMapFunc;
using ch::syst::SystMap;
using ch::syst::SystMapFunc;
using ch::syst::bin;
using json = nlohmann::json;
typedef std::map<std::string, std::vector<std::string> > channelmap; 
using std::vector;
using std::string;
using std::map;
using std::pair;

//how a configured systematic is shared between split categories (e.g. Run2/Run3)
enum class SplitCorrelation {
	Correlated,   //one nuisance name for the matching bins of every split
	Uncorrelated  //one nuisance per split, named <name>_<split>
};

struct yamlSys{
        public:
                yamlSys(YAML::Node syst) :
                _init_val(-1){
                        _type = syst["type"].as<string>();
                        _name = syst["name"].as<string>();
                        _init_val = syst["init_val"].as<double>();
			if(syst["init_err"])
				_init_err = syst["init_err"].as<double>();
			if(!syst["bins"])
				_bins = {};
			else{
                        	_bins = syst["bins"].as<vector<string>>();
			}
			if(!syst["procs"])
				_procs = {"bkg"};
			else
				_procs = syst["procs"].as<vector<string>>();
			//defaults to uncorrelated so a split fit does not assume a shared nuisance unless asked
			_split_correlation = SplitCorrelation::Uncorrelated;
			if(syst["split_correlation"]){
				string corr = syst["split_correlation"].as<string>();
				if(corr == "correlated")
					_split_correlation = SplitCorrelation::Correlated;
				else if(corr != "uncorrelated")
					throw std::runtime_error("Systematic '" + _name + "' has invalid split_correlation '" + corr + "'; expected 'correlated' or 'uncorrelated'");
			}
                };
                string _type;
		string _name;
                double _init_val;
		double _init_err = -999;
                vector<string> _bins;
		vector<string> _procs;
		SplitCorrelation _split_correlation;
};

//a process whose ABCD target-channel yield is templated from a source channel:
//rate(target bin i) = yield(source bin i) * scale_<target>_<process>, with the scale initialized to the transfer factor
struct ABCDTemplateProcess{
	string source_channel;
	string target_channel;
	string process;
	bool auto_transfer_factor = true; //auto: min over matching bins i of yield(target i)/yield(source i)
	double transfer_factor = 1.;
};

//decomposition of one target-channel bin of one split into its templated part and the ABCD residual:
//	target_yield = template_yield + residual_yield,  template_yield = transfer_factor * source_yield
struct ABCDTemplateBinResult{
	string source_bin;
	string target_bin;
	string template_process; //process added by the template
	string abcd_process;     //ABCD process of the target channel, initialized from the residual
	double source_yield = 0.;
	double target_yield = 0.;
	double transfer_factor = 0.;
	double template_yield = 0.;
	double residual_yield = 0.;
};

//one configured template evaluated in one split
struct ABCDTemplateSplitResult{
	ABCDTemplateProcess config;
	string split;
	double transfer_factor = 0.;
	vector<ABCDTemplateBinResult> bins;
};


class BuildFit{
	public:
		//takes in fit config yaml file
		BuildFit(string infile = "");
		ch::CombineHarvester cb{};
	
		void PrepFit(JSONFactory* j, string signalPoint, vector<string> datakeys = {});
		void SetObservations();
		void SetSignalRates();
		void BuildShapeTransferFit();
		void BuildShapeTransferFit(const string& split);
		void BuildABCDFitSingleBin();
		void BuildABCDFit();
		void DoSystematics();
		void DoSystematics(const string& split);
		void WriteDatacard(string datacard_dir, bool verbose = false);
		double SumObs(const string& bin){
			double obs = 0;
                	for(auto it = _obs_rates[bin].begin(); it != _obs_rates[bin].end(); it++){
				//skip processes that don't contribute to this bin
				if(it->second < 1e-10)
					continue;
				//std::cout << "SumObs - bin " << bin << " proc " << it->first << " yield " << it->second << std::endl;	
                        	obs += it->second;
			}
			return obs;
		}

		ch::Categories BuildCats(JSONFactory* j);
		void BuildCatsSubset(std::set<string> categories, ch::Categories& retcats); //returns subset of cats from overall _cats
		std::map<std::string, float> BuildAsimovData(JSONFactory* j);
        	std::vector<std::string> GetBkgProcs(JSONFactory* j);
		std::vector<std::string> GetDataProcs(JSONFactory* j);
		std::vector<std::string> ExtractSignalDetails( std::string signalPoint);
		std::vector<std::string> GetBinSet( JSONFactory* j);
		std::map<std::string, float> LoadObservations(JSONFactory* j);
		double GetStatFracError(JSONFactory* j, std::string binName, std::vector<std::string> bkgprocs );
		std::map<std::string, float> LoadDataProcesses(JSONFactory* j, std::vector<std::string> dataKeys);


		void BuildAsimovFit(JSONFactory* j, std::string signaPoint, std::string datacard_dir);
		void BuildABCDFit(JSONFactory* j, std::string signalPoint, std::string datacard_dir, std::vector<std::string> ABCDbins);
		void BuildPseudoShapeTemplateFit(JSONFactory* j, JSONFactory* jup, JSONFactory* jdn, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void Build9binFitMC(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void Build9binFitData(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);
		void BuildMultiChannel9bin(JSONFactory* j, std::string signalPoint, std::string datacard_dir, channelmap channelMap);

		void AddTemplateProcessABCD(string src_ch, string target_ch, double tf = -999, string proc = "");
		void AddTemplateProcessABCD(string src_ch, string target_ch, string split, double tf = -999, string proc = "");

		std::vector<std::string> sigkeys = { "gogoZ", "gogoG", "gogoGZ", "sqsqZ", "sqsqG", "sqsqGZ" };
		//aggregate data key (BFI sums every data era in a bin into it); the only key used as data-driven bkg
		std::vector<std::string> datakeys = { "data" };
		//per-era data keys (MET16, MET23, ...) that BFI writes next to the aggregate; never a bkg process
		std::vector<std::string> dataeraprefixes = { "MET", "JetMET", "DisplacedJet" };
		bool IsDataKey(const std::string& key) const{
			for(const auto& k : datakeys)
				if(key == k) return true;
			return false;
		}
		bool IsDataEraKey(const std::string& key) const{
			for(const auto& p : dataeraprefixes)
				if(key.compare(0, p.size(), p) == 0) return true;
			return false;
		}

		string GetFitName(){ return _fitname; }

		//split labels the model is built for: {""} unsplit, {"Run2","Run3"} for a run split
		vector<string> ActiveSplitLabels() const{ return DataSplit::SplitLabels(_datasplit); }

		//get process for bin (includes binidx)
		string getProcess(string crbin){
			string crch = getChannel(crbin);
			for(auto it = _abcd_ch_ass.begin(); it != _abcd_ch_ass.end(); it++){
				string srch = it->first;
				for(auto iit = _abcd_ch_ass[srch].begin(); iit != _abcd_ch_ass[srch].end(); iit++){
					string proc = iit->first;
					vector<string> crchs = _abcd_ch_ass[srch][proc];
					if(find(crchs.begin(), crchs.end(), crch) != crchs.end())
						return proc;
				}
			}
			return _bkg_proc;
		}

	private:
		channelmap _shape_ch_ass; //channel association for shape transfer fit
		channelmap _shape_bin_ass; //bin associations for each channel
		map<pair<string,string>,double> _shape_tf_ass; //associates channel mappings to transfer factor, _shape_tf_ass[(anchor,buoy)] = tf
		channelmap _abcd_bin_ass; //SR (key) to B, C, D (vals) for ABCD fit
		//channelmap _abcd_ch_ass; //if channels are connected between ABCD fits
		map<string,channelmap> _abcd_ch_ass; //maps like _abcd_ch_ass[sr_ch][proc] = {cr_bins}
		vector<yamlSys> _systs; //extra systematics to connect channels, etc
		ch::Categories _cats;
		map<string, int> _invcats; 
		bool _asimov; //sets observation to expected yields
		bool _datadriven; //uses data as 'bkg procs'
		bool _preserve_background_processes = false; //keeps MC backgrounds as separate Combine processes
		bool _direct_mc_backgrounds_inserted = false;
		//std::map<std::string, float> _obs_rates;
		std::map<string, std::map<string, double>> _obs_rates; //map for obs_rates[bin][proc] 
		std::vector<std::string> _bkgprocs;
		std::vector<std::string> _signalDetails;
		json _yields;
		//logical bins as written in the fit config (never carry a split suffix)
		std::set<string> _bins_superset;
		std::set<string> _bins_superset_abcd;
		std::set<string> _bins_superset_shape;
		//concrete fit categories: logical bins expanded over ActiveSplitLabels()
		std::set<string> _fit_bins;
		std::set<string> _fit_bins_abcd;
		DataSplit::DataSplitMode _datasplit = DataSplit::DataSplitMode::None;
		vector<ABCDTemplateProcess> _abcd_templates;
		//template decompositions of the current model, computed before any ABCD rateParam is created
		vector<ABCDTemplateSplitResult> _abcd_template_results;
		map<string, ABCDTemplateBinResult> _abcd_template_bins; //concrete target bin -> its decomposition
		map<string, string> _abcd_proc_denom_ch;
		string _fitname;
		string _signalPoint;
		map<string, string> _shape_anchor_bins;
		string _bkg_proc = "bkg";
		bool _rateparam_railguard_enabled = true;
		double _rateparam_railguard_min = 1.0e-6;
		double _rateparam_railguard_max = 1000000.0;
		bool _rateparam_floor_zero_init = true;
		bool _yearsplit = false;
		bool _runsplit = false;
		//get total yield over all processes for a given bin
		double getTotYield(string bin){
			double bin_tot_yield = 0;
                        for(auto proc : _bkgprocs){
				std::cout << "getTotYield - bin " << bin << " proc " << proc << std::endl;
                                bin_tot_yield += GetYieldValueOrZero(bin, proc, 1, "getTotYield");
                        }
			return bin_tot_yield;
		}

		//logical anchor-index bin of a buoy channel (its CR bin)
		string GetBuoyBin(string buoych){
			//look up the index before replace() mutates buoych (operand order of + is unspecified)
			string anchor_idx = _shape_anchor_bins[buoych];
			return buoych.replace(buoych.find("SR"),2,"CR")+anchor_idx;
		}

		//two-character bin index, ignoring any split suffix
		string getBinIdx(const string& binname) const{ return DataSplit::BinIdx(binname); }
		//channel of a bin, ignoring any split suffix
		string getChannel(const string& binname) const{ return DataSplit::Channel(binname); }

		std::set<string> ExpandToSplits(const std::set<string>& logical_bins) const;
		void ValidateInputBins(const json& yields) const;
		void ParseABCDTemplates(const YAML::Node& node);
		void BuildShapeTransferFitForSplit(const string& split);
		void BuildABCDConstraintsForSplit(const string& split);
		string ABCDProcessOfChannel(const string& cr_ch) const;
		double DataDrivenYield(const string& bin, const string& context) const;
		//template decomposition of one split (pure calculation, does not touch the model); throws on a negative residual
		ABCDTemplateSplitResult ComputeABCDTemplate(const ABCDTemplateProcess& tmpl, const string& split) const;
		void ComputeABCDTemplates();
		double ABCDControlYield(const string& crbin) const;
		void AddABCDTemplateProcess(const ABCDTemplateSplitResult& result);
		void ValidateSystematics() const;
		string SystematicNameForSplit(const yamlSys& syst, const string& split) const;

		void sumBkgs();
		void InsertDirectMCBackgroundProcesses();
		double GetYieldValue(const string& bin, const string& proc, int index, const string& context) const;
		double GetYieldValueOrZero(const string& bin, const string& proc, int index, const string& context) const;
		std::vector<std::string> FitBackgroundProcesses() const;
		std::vector<std::string> ExpandConfiguredProcesses(const std::vector<std::string>& configured) const;
		double GuardRateParamInit(double value) const;
		std::string ResolveRateParamName(const std::string& name, const std::string& bin, const std::string& proc) const;
		void SetRateParamRange(const std::string& name, const std::vector<std::string>& bins, const std::vector<std::string>& procs);
		void AddRateParam(const std::vector<std::string>& procs, const std::vector<std::string>& bins, const std::string& name, double init);
		ch::Process create_proc(string mass, string analysis, string era, string channel, string proc, pair<int, string> bininfo, bool signal, double rate){
			ch::Process newproc;
                        newproc.set_mass(mass);
                        newproc.set_analysis(analysis);
                        newproc.set_era(era);
                        newproc.set_channel(channel);
                        newproc.set_process(proc);
			newproc.set_bin_id(bininfo.first);
			newproc.set_bin(bininfo.second);
			newproc.set_process(proc);
			newproc.set_signal(signal);
			newproc.set_rate(rate);
			return newproc;
		};	
};
#endif
