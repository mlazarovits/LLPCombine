

#include "SampleTool.h"
#include "BuildFitInput.h"
#include "JSONFactory.h"
#include "ConfigParser.h"
#include "ArgumentParser.h"
#include <iostream>
#include <sys/stat.h>
#include <errno.h>

/* needs integrated for data process
<<<<<<< HEAD
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
=======
*/
// Function to process a single configuration file
int ProcessSingleConfig(const std::string& config_file, const ProgramOptions& options) {
	// Load configuration
	ConfigParser configParser;
	if (!configParser.LoadConfig(config_file)) {
		std::cerr << "Error: Failed to load configuration from: " << config_file << std::endl;
		return 1;
	}
	
	const AnalysisConfig& config = configParser.GetConfig();
	
	// Override config with command-line options if provided
	double luminosity = (options.luminosity > 0) ? options.luminosity : config.luminosity;
	int verbosity = (options.verbosity >= 0) ? options.verbosity : config.verbosity;
	std::string output_dir = options.output_dir.empty() ? config.output_dir : options.output_dir;
	
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
		std::cout << "Processing: " << config.name << " -> " << config.output_json << std::endl;
	}
	
	// Dry run - just validate and exit
	if (options.dry_run || config.dry_run) {
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
	stringlist bkglist(config.backgrounds.begin(), config.backgrounds.end());
	stringlist siglist(config.signals.begin(), config.signals.end());
	stringlist datalist(config.data.begin(), config.data.end());	
	
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

	BFI->LoadData_byMap(ST->DataDict);
	BFI->LoadBkg_byMap(ST->BkgDict, luminosity);
	BFI->LoadSig_byMap(ST->SigDict, luminosity);
	BFI->BuildScaledEvtWt(luminosity);

		
	// Create analysis bins from configuration
	for (const auto& bin : config.bins) {
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
	summap sumResults_S = BFI->SumRegions("evtwt", BFI->sig_filtered_dataframes);
	summap sumResults_obs = BFI->SumRegions("evtwt", BFI->data_filtered_dataframes);
	//summap sumResults = BFI->SumRegions("LumiEvtWt", BFI->bkg_filtered_dataframes);
        //summap sumResults_S = BFI->SumRegions("evtFillWgt", BFI->sig_filtered_dataframes);
        //summap sumResults_obs = BFI->SumRegions("LumiEvtWt", BFI->data_filtered_dataframes);

	//book error sums	
		

	// Initiate action
	BFI->ReportRegions(verbosity > 2 ? 1 : 0);
	
	// Compute errors and report bins
	errormap errorResults = BFI->ComputeStatError(countResults, BFI->bkg_evtwt);
	errormap errorResults_S = BFI->ComputeStatError(countResults_S, BFI->sig_evtwt);
	errormap errorResults_obs = BFI->ComputeStatError(countResults_obs, BFI->data_evtwt);

	// Aggregate maps into more easily useable classes
	BFI->ConstructBkgBinObjects(countResults, sumResults, errorResults);
	BFI->AddSigToBinObjects(countResults_S, sumResults_S, errorResults_S, BFI->analysisbins);
	BFI->AddDataToBinObjects( countResults_obs, sumResults_obs, errorResults_obs, BFI->analysisbins);

	if (verbosity > 0 && !options.batch_mode) {
		BFI->PrintBins(verbosity > 1 ? 1 : 0);
	}
	
	// Write output JSON
	std::string output_path = output_dir + "/" + config.output_json;
	JSONFactory* json = new JSONFactory(BFI->analysisbins);
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
	
	// Show batch mode status
	if (options.batch_mode && options.verbosity >= 0) {
		std::cout << "=== LLPCombine Batch Mode ===" << std::endl;
		std::cout << "Processing " << options.config_files.size() << " configuration files:" << std::endl;
		for (size_t i = 0; i < options.config_files.size(); ++i) {
			std::cout << "  " << (i+1) << ". " << options.config_files[i] << std::endl;
		}
		std::cout << std::endl;
	}
	
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
	
	// Print batch summary
	if (options.batch_mode) {
		std::cout << "\n=== Batch Processing Summary ===" << std::endl;
		std::cout << "Successfully processed: " << total_processed << "/" << options.config_files.size() << " files" << std::endl;
		if (total_failed > 0) {
			std::cout << "Failed: " << total_failed << " files" << std::endl;
		}
	}
	
	return (total_failed > 0) ? 1 : 0;
}
