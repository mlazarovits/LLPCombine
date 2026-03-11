#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

#include <string>
#include <vector>
#include <map>

struct ProgramOptions {
    std::vector<std::string> config_files;  // Changed to support multiple files
    std::string output_dir;
    double luminosity;
    int verbosity;
    bool help;
    bool dry_run;
    bool version;
    bool unblind;
    bool batch_mode;  // New: indicates batch processing

    ProgramOptions() : 
        output_dir(""),
        luminosity(-1.0),  // -1 indicates not set
        verbosity(-1),     // -1 indicates not set
        help(false),
        dry_run(false),
        version(false),
	unblind(false),
        batch_mode(false) {}
};

class ArgumentParser {
public:
    ArgumentParser();
    ~ArgumentParser();
    
    ProgramOptions Parse(int argc, char* argv[]);
    void PrintHelp(const std::string& program_name) const;
    void PrintVersion() const;
    
private:
    std::string GetNextArg(const std::vector<std::string>& args, size_t& index) const;
    bool IsFlag(const std::string& arg) const;
    
    // New methods for batch processing
    std::vector<std::string> ExpandConfigInputs(const std::vector<std::string>& inputs) const;
    std::vector<std::string> ReadConfigList(const std::string& list_file) const;
    std::vector<std::string> ExpandGlobPattern(const std::string& pattern) const;
    bool IsListFile(const std::string& filename) const;
    bool IsGlobPattern(const std::string& input) const;
};

#endif
