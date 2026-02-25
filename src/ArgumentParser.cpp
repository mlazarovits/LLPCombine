#include "ArgumentParser.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <glob.h>
#include <sys/stat.h>

ArgumentParser::ArgumentParser() {}

ArgumentParser::~ArgumentParser() {}

ProgramOptions ArgumentParser::Parse(int argc, char* argv[]) {
    ProgramOptions options;
    std::vector<std::string> args;
    
    // Convert to vector for easier handling
    for (int i = 1; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }
    
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        
        if (arg == "-h" || arg == "--help") {
            options.help = true;
        }
        else if (arg == "-v" || arg == "--version") {
            options.version = true;
        }
        else if (arg == "-c" || arg == "--config") {
            options.config_files.push_back(GetNextArg(args, i));
        }
        else if (arg == "-o" || arg == "--output-dir") {
            options.output_dir = GetNextArg(args, i);
        }
        else if (arg == "-l" || arg == "--luminosity") {
            std::string lumi_str = GetNextArg(args, i);
            try {
                options.luminosity = std::stod(lumi_str);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid luminosity value: " << lumi_str << std::endl;
                throw;
            }
        }
        else if (arg == "--verbosity") {
            std::string verb_str = GetNextArg(args, i);
            try {
                options.verbosity = std::stoi(verb_str);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid verbosity value: " << verb_str << std::endl;
                throw;
            }
        }
        else if (arg == "--dry-run") {
            options.dry_run = true;
        }
        else if (arg == "--unblind") {
            options.unblind = true;
        }
        else if (arg.front() == '-') {
            std::cerr << "Warning: Unknown option: " << arg << std::endl;
        }
        else {
            // Positional argument - add to config files list
            options.config_files.push_back(arg);
        }
    }
    
    // Expand config inputs (handle lists, globs, etc.)
    if (!options.config_files.empty()) {
        options.config_files = ExpandConfigInputs(options.config_files);
        options.batch_mode = (options.config_files.size() > 1);
    }
    
    return options;
}

std::string ArgumentParser::GetNextArg(const std::vector<std::string>& args, size_t& index) const {
    if (index + 1 >= args.size()) {
        throw std::runtime_error("Option " + args[index] + " requires an argument");
    }
    
    ++index;
    const std::string& next_arg = args[index];
    
    if (IsFlag(next_arg)) {
        throw std::runtime_error("Option " + args[index-1] + " requires an argument, got flag: " + next_arg);
    }
    
    return next_arg;
}

bool ArgumentParser::IsFlag(const std::string& arg) const {
    return !arg.empty() && arg.front() == '-';
}

void ArgumentParser::PrintHelp(const std::string& program_name) const {
    std::cout << "Usage: " << program_name << " [OPTIONS] [CONFIG_FILES...]\n\n";
    std::cout << "LLPCombine - Long-Lived Particle Analysis Framework\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message and exit\n";
    std::cout << "  -v, --version           Show version information and exit\n";
    std::cout << "  -c, --config FILE       Configuration file (YAML format, can be used multiple times)\n";
    std::cout << "  -o, --output-dir DIR    Output directory for results\n";
    std::cout << "  -l, --luminosity LUMI   Integrated luminosity (fb^-1)\n";
    std::cout << "      --verbosity LEVEL   Verbosity level (0-3)\n";
    std::cout << "      --dry-run           Validate configuration without processing\n\n";
    std::cout << "      --unblind           Unblinds signal regions\n\n";
    std::cout << "Batch Processing Support:\n";
    std::cout << "  Multiple configs:       " << program_name << " config1.yaml config2.yaml config3.yaml\n";
    std::cout << "  Config list file:       " << program_name << " configs.txt (or .list)\n";
    std::cout << "  Glob patterns:          " << program_name << " \"config/*.yaml\"\n";
    std::cout << "  Mixed inputs:           " << program_name << " config1.yaml configs.txt \"more/*.yaml\"\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " config/analysis.yaml\n";
    std::cout << "  " << program_name << " --config config/analysis.yaml --output-dir results/\n";
    std::cout << "  " << program_name << " config1.yaml config2.yaml config3.yaml\n";
    std::cout << "  " << program_name << " \"config/*.yaml\" --verbosity 1\n";
    std::cout << "  " << program_name << " analysis_configs.txt\n";
    std::cout << "  " << program_name << " --dry-run \"config/*.yaml\"\n\n";
    std::cout << "For more information, see the documentation.\n";
}

void ArgumentParser::PrintVersion() const {
    std::cout << "LLPCombine version 1.0.0\n";
    std::cout << "Built with ROOT and CMS Combine support\n";
}

std::vector<std::string> ArgumentParser::ExpandConfigInputs(const std::vector<std::string>& inputs) const {
    std::vector<std::string> expanded_configs;
    
    for (const auto& input : inputs) {
        if (IsListFile(input)) {
            // Read config files from list
            auto list_configs = ReadConfigList(input);
            expanded_configs.insert(expanded_configs.end(), list_configs.begin(), list_configs.end());
        } else if (IsGlobPattern(input)) {
            // Expand glob pattern
            auto glob_configs = ExpandGlobPattern(input);
            expanded_configs.insert(expanded_configs.end(), glob_configs.begin(), glob_configs.end());
        } else {
            // Regular config file
            expanded_configs.push_back(input);
        }
    }
    
    return expanded_configs;
}

std::vector<std::string> ArgumentParser::ReadConfigList(const std::string& list_file) const {
    std::vector<std::string> configs;
    std::ifstream file(list_file);
    
    if (!file.is_open()) {
        std::cerr << "Warning: Cannot open list file: " << list_file << std::endl;
        return configs;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace and skip empty lines and comments
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (!line.empty() && line.front() != '#') {
            configs.push_back(line);
        }
    }
    
    return configs;
}

std::vector<std::string> ArgumentParser::ExpandGlobPattern(const std::string& pattern) const {
    std::vector<std::string> matches;
    glob_t glob_result;
    
    int ret = glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result);
    
    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            matches.push_back(std::string(glob_result.gl_pathv[i]));
        }
        globfree(&glob_result);
    } else {
        std::cerr << "Warning: Glob pattern '" << pattern << "' matched no files" << std::endl;
    }
    
    // Sort matches for consistent ordering
    std::sort(matches.begin(), matches.end());
    
    return matches;
}

bool ArgumentParser::IsListFile(const std::string& filename) const {
    // Check file extension
    if (filename.length() < 4) return false;
    
    std::string ext = filename.substr(filename.length() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    return (ext == ".txt" || ext == ".list");
}

bool ArgumentParser::IsGlobPattern(const std::string& input) const {
    // Check for glob characters
    return (input.find('*') != std::string::npos || 
            input.find('?') != std::string::npos ||
            input.find('[') != std::string::npos);
}
