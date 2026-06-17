#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"
#include "ConfigParser.h"
#include "ArgumentParser.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include <errno.h>

// Convert a ctau value to a filename-safe tag using the Xp convention:
//   50.0  -> "50"
//   0.01  -> "0p01"
//   1.5   -> "1p5"
static std::string CtauToTag(double ctau) {
    if (ctau == std::floor(ctau)) {
        return std::to_string(static_cast<long long>(ctau));
    }
    std::ostringstream oss;
    oss << ctau;
    std::string s = oss.str();
    std::replace(s.begin(), s.end(), '.', 'p');
    return s;
}

// Function to process a single configuration file
int ProcessSingleConfig(const std::string& config_file, const ProgramOptions& options) {
	// Load configuration
	ConfigParser configParser;
	if (!configParser.LoadConfig(config_file)) {
		std::cerr << "Error: Failed to load configuration from: " << config_file << std::endl;
		return 1;
	}
	
	AnalysisConfig cfg = configParser.GetConfig();

	// Override config with command-line options if provided
	double luminosity = (options.luminosity > 0) ? options.luminosity : cfg.luminosity;
	int verbosity = (options.verbosity >= 0) ? options.verbosity : cfg.verbosity;
	std::string output_dir = options.output_dir.empty() ? cfg.output_dir : options.output_dir;

	// Apply CLI ctau overrides. Capture whether YAML already configured reweighting before
	// applying overrides so we know whether to auto-suffix the output filename below.
	bool yaml_had_reweighting = (cfg.sampleLifetime > 0);
	if (options.source_ctau > 0) cfg.sampleLifetime = options.source_ctau;
	if (options.target_ctau > 0) cfg.targetLifetime = options.target_ctau;

	// When --target-ctau is supplied on the CLI and the YAML did not already configure
	// reweighting, suffix the output JSON name with _ctauN so that runs at different ctau
	// values don't overwrite each other (e.g. "MyAnalysis.json" -> "MyAnalysis_ctau30.json").
	// NOTE: The signal process KEY inside the JSON (e.g. "gogoGZ_2300_1300_1000_50") comes
	// from the MC filename via BFTool::GetSignalTokens and reflects the *generated* ctau, not
	// the target ctau. BF.x therefore names datacard directories after the original ctau.
	// A ctau scan pipeline must rename those directories to carry the correct target ctau label.
	if (options.target_ctau > 0 && !yaml_had_reweighting) {
		std::string suffix = "_ctau" + CtauToTag(options.target_ctau);
		size_t dot = cfg.output_json.rfind('.');
		if (dot != std::string::npos)
			cfg.output_json = cfg.output_json.substr(0, dot) + suffix + cfg.output_json.substr(dot);
		else
			cfg.output_json += suffix;
	}

	// Apply CLI BR overrides. When --target-zrate/--target-grate are given and the YAML did
	// not already have a decayWeights block, auto-suffix the output JSON so runs at different
	// BR points don't overwrite each other (e.g. "MyAnalysis.json" -> "MyAnalysis_Z100G0.json").
	bool yaml_had_decay_weights = (cfg.targetZrate >= 0 || cfg.targetGrate >= 0);
	if (options.source_zrate >= 0) cfg.sampleZrate = options.source_zrate;
	if (options.source_grate >= 0) cfg.sampleGrate = options.source_grate;
	if (options.target_zrate >= 0) cfg.targetZrate = options.target_zrate;
	if (options.target_grate >= 0) cfg.targetGrate = options.target_grate;
	// SMS samples are generated at equal Z/photon BR (50/50). Default to that
	// when neither the YAML nor the CLI specified target rates.
	if (cfg.targetZrate < 0 && cfg.targetGrate < 0) {
		cfg.targetZrate = 0.5;
		cfg.targetGrate = 0.5;
	}
	// Default source rates to 0.5 when not explicitly provided.
	if (cfg.sampleZrate < 0) cfg.sampleZrate = 0.5;
	if (cfg.sampleGrate < 0) cfg.sampleGrate = 0.5;
	if (!yaml_had_decay_weights) {
		int zpct = static_cast<int>(std::round(cfg.targetZrate * 100));
		int gpct = static_cast<int>(std::round(cfg.targetGrate * 100));
		std::string suffix = "_Z" + std::to_string(zpct) + "G" + std::to_string(gpct);
		size_t dot = cfg.output_json.rfind('.');
		if (dot != std::string::npos)
			cfg.output_json = cfg.output_json.substr(0, dot) + suffix + cfg.output_json.substr(dot);
		else
			cfg.output_json += suffix;
	}
	
	// Print configuration if verbose
	if (verbosity > 0 && !options.batch_mode) {
		std::cout << "=== LLPCombine Analysis ===" << std::endl;
		configParser.PrintConfig();
		std::cout << "\nCommand-line overrides:" << std::endl;
		if (options.luminosity > 0) std::cout << "  Luminosity: " << luminosity << " fb^-1" << std::endl;
		if (options.verbosity >= 0) std::cout << "  Verbosity: " << verbosity << std::endl;
		if (!options.output_dir.empty()) std::cout << "  Output dir: " << output_dir << std::endl;
		std::cout << std::endl;
	} else if (options.batch_mode && verbosity > 0) {
		std::cout << "Processing: " << cfg.name << " -> " << cfg.output_json << std::endl;
	}

	// Dry run - just validate and exit
	if (options.dry_run || cfg.dry_run) {
		if (verbosity > 0) {
			std::cout << "Dry run completed - configuration " << config_file << " is valid." << std::endl;
		}
		return 0;
	}
	
	// Ensure output directory exists
	struct stat st = {0};
	if (stat(output_dir.c_str(), &st) == -1) {
		if (mkdir(output_dir.c_str(), 0755) != 0) {
			std::cerr << "Error creating output directory " << output_dir << ": " << strerror(errno) << std::endl;
			return 1;
		}
	}
	
	// Initialize SampleTool with configuration
	SampleTool* ST = new SampleTool();
	
	// Convert vectors to the expected stringlist format
	stringlist bkglist(cfg.backgrounds.begin(), cfg.backgrounds.end());
	stringlist siglist(cfg.signals.begin(), cfg.signals.end());
	stringlist datalist(cfg.data.begin(), cfg.data.end());
	
	ST->LoadBkgs(bkglist);
	ST->LoadSigs(siglist);
	ST->LoadData( datalist);

	if (verbosity > 1 && !options.batch_mode) {
		ST->PrintDict(ST->BkgDict);
		ST->PrintDict(ST->DataDict);
		ST->PrintDict(ST->SigDict);
		ST->PrintKeys(ST->SignalKeys);
	}
	
	// Initialize BuildFitInput
	BuildFitInput* BFI = new BuildFitInput();
	BFI->SetUnblind(options.unblind);

	BFI->LoadData_byMap(ST->DataDict);
	BFI->LoadBkg_byMap(ST->BkgDict, luminosity);
	BFI->LoadSig_byMap(ST->SigDict, luminosity);
	//BFI->BuildScaledEvtWt(luminosity);

	//Load new weights for reweighting scenarios
	BFI->BuildReweights( cfg );

	// Create analysis bins from configuration
	for (const auto& bin : cfg.bins) {
		std::string combined_cuts = configParser.GetCombinedCuts(bin.name);
		
		if (verbosity > 1 && !options.batch_mode) {
			std::cout << "Creating bin: " << bin.name << std::endl;
			std::cout << "  Description: " << bin.description << std::endl;
			std::cout << "  Cuts: " << combined_cuts << std::endl;
		}
		
		BFI->FilterRegions(bin.name, combined_cuts);
		BFI->CreateBin(bin.name);
	}


	// Book operations
	countmap countResults = BFI->CountRegions(BFI->bkg_filtered_dataframes);
	countmap countResults_S = BFI->CountRegions(BFI->sig_filtered_dataframes);
	countmap countResults_obs = BFI->CountRegions(BFI->data_filtered_dataframes);

	summap sumResults = BFI->SumRegions("evtwt", BFI->bkg_filtered_dataframes);
	summap sumResults_S = BFI->SumRegions("evtrwt", BFI->sig_filtered_dataframes);
	summap sumResults_obs = BFI->SumRegions("evtwt", BFI->data_filtered_dataframes);
	//summap sumResults = BFI->SumRegions("LumiEvtWt", BFI->bkg_filtered_dataframes);
        //summap sumResults_S = BFI->SumRegions("evtFillWgt", BFI->sig_filtered_dataframes);
        //summap sumResults_obs = BFI->SumRegions("LumiEvtWt", BFI->data_filtered_dataframes);

	//new evtwt error calculation
	summap errorsumResults = BFI->SumRegions("evtwt2", BFI->bkg_filtered_dataframes);
    summap errorsumResults_S = BFI->SumRegions("evtrwt2", BFI->sig_filtered_dataframes);


	//book error sums	
		

	// Initiate action
	BFI->ReportRegions(verbosity > 2 ? 1 : 0);
	
	// Compute errors and report bins
	//errormap errorResults = BFI->ComputeStatError(countResults, BFI->bkg_evtwt);
	//errormap errorResults_S = BFI->ComputeStatError(countResults_S, BFI->sig_evtwt);
	errormap errorResults = BFI->ComputeStatError(errorsumResults);
    errormap errorResults_S = BFI->ComputeStatError(errorsumResults_S);
	errormap errorResults_obs = BFI->ComputeStatError(countResults_obs, BFI->data_evtwt);
	//errormap errorResults_obs = BFI->ComputeStatError(countResults_obs, 1);


	// Aggregate maps into more easily useable classes
	BFI->ConstructBkgBinObjects(countResults, sumResults, errorResults);
	BFI->AddSigToBinObjects(countResults_S, sumResults_S, errorResults_S, BFI->analysisbins);
	if(cfg.mc_closure){
		BFI->AddMCClosureDataToBinObjects(BFI->analysisbins);
	}
	else{
		BFI->AddDataToBinObjects( countResults_obs, sumResults_obs, errorResults_obs, BFI->analysisbins);
	}

	if (verbosity > 0 && !options.batch_mode) {
		BFI->PrintBins(verbosity > 1 ? 1 : 0);
	}

	// Write output JSON
	std::string output_path = output_dir + "/" + cfg.output_json;
	JSONFactory* json = new JSONFactory(BFI->analysisbins, cfg, cfg.mc_closure, cfg.mc_closure_background_mode);
	json->WriteJSON(output_path);
	
	if (verbosity > 0) {
		if (options.batch_mode) {
			std::cout << "  -> " << output_path << std::endl;
		} else {
			std::cout << "Results written to: " << output_path << std::endl;
		}
	}
	
	// Cleanup
	delete json;
	delete BFI;
	delete ST;
	
	return 0;
}

int main(int argc, char* argv[]) {
	// Parse command-line arguments
	ArgumentParser argParser;
	ProgramOptions options;
	
	try {
		options = argParser.Parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		argParser.PrintHelp(argv[0]);
		return 1;
	}
	
	// Handle help and version requests
	if (options.help) {
		argParser.PrintHelp(argv[0]);
		return 0;
	}
	
	if (options.version) {
		argParser.PrintVersion();
		return 0;
	}
	
	// Require configuration file(s)
	if (options.config_files.empty()) {
		std::cerr << "Error: At least one configuration file required. Use -c/--config or provide as positional argument." << std::endl;
		argParser.PrintHelp(argv[0]);
		return 1;
	}

	//do unblinding check
	if(options.unblind){
	        string proceed;
                std::cout << "WARNING unblinding analysis!!!!!!!" << std::endl;
                std::cout << "Are you sure you want to proceed? y/n" << std::endl;
                std:: cin >> proceed;
                if(proceed != "y")
			return 1;
	};

/*	
	// Show batch mode status
	if (options.batch_mode && options.verbosity >= 0) {
		std::cout << "=== LLPCombine Batch Mode ===" << std::endl;
		std::cout << "Processing " << options.config_files.size() << " configuration files:" << std::endl;
		for (size_t i = 0; i < options.config_files.size(); ++i) {
			std::cout << "  " << (i+1) << ". " << options.config_files[i] << std::endl;
		}
		std::cout << std::endl;
	}
*/	
	// Process all configuration files
	int total_processed = 0;
	int total_failed = 0;
	
	for (const auto& config_file : options.config_files) {
		int result = ProcessSingleConfig(config_file, options);
		if (result == 0) {
			total_processed++;
		} else {
			total_failed++;
			if (!options.batch_mode) {
				// In single-file mode, exit immediately on failure
				return result;
			}
		}
	}
/*	
	// Print batch summary
	if (options.batch_mode) {
		std::cout << "\n=== Batch Processing Summary ===" << std::endl;
		std::cout << "Successfully processed: " << total_processed << "/" << options.config_files.size() << " files" << std::endl;
		if (total_failed > 0) {
			std::cout << "Failed: " << total_failed << " files" << std::endl;
		}
	}
*/	
	return (total_failed > 0) ? 1 : 0;
}
