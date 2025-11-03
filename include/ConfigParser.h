#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

struct BinConfig {
    std::string name;
    std::string description;
    std::vector<std::string> cuts;
};

struct SystematicConfig {
    std::string name;
    std::string type;
    double value;
    std::vector<std::string> processes;
};

struct AnalysisConfig {
    std::string name;
    double luminosity;
    std::string output_json;
    std::string output_dir;
    
    std::vector<std::string> backgrounds;
    std::vector<std::string> signals;
    
    std::vector<BinConfig> bins;
    std::vector<SystematicConfig> systematics;
    
    // Runtime options
    int verbosity;
    bool parallel;
    bool dry_run;
};

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser();
    
    bool LoadConfig(const std::string& config_file);
    const AnalysisConfig& GetConfig() const { return config_; }
    
    // Utility methods
    std::string GetCombinedCuts(const std::string& bin_name) const;
    void PrintConfig() const;
    bool ValidateConfig() const;
    
private:
    AnalysisConfig config_;
    bool LoadYAML(const std::string& config_file);
    void SetDefaults();
};

#endif