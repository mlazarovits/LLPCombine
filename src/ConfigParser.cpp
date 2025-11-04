#include "ConfigParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Simple YAML parser implementation
// For production use, consider yaml-cpp library
class SimpleYAMLParser {
public:
    std::map<std::string, std::string> values;
    std::map<std::string, std::vector<std::string>> lists;
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::map<std::string, std::vector<std::string>> anchors;
    
    bool parse(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open config file: " << filename << std::endl;
            return false;
        }
        
        // First pass: collect all anchors
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        
        // Parse anchors first
        parseAnchors(lines);
        
        // Second pass: normal parsing with anchor expansion
        std::string current_section = "";
        std::string current_subsection = "";
        std::string current_anchor_key = "";  // Track current anchor being defined
        
        for (const std::string& line_raw : lines) {
            line = line_raw;
            // Check indentation level BEFORE trimming
            size_t indent = 0;
            for (char c : line) {
                if (c == ' ') indent++;
                else break;
            }
            
            // Remove comments and trim whitespace
            size_t comment_pos = line.find('#');
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (line.empty()) continue;
            
            if (indent == 0) {
                // Top level section
                if (line.back() == ':') {
                    std::string section_key = line.substr(0, line.length() - 1);
                    
                    // Check if this is an anchor definition at top level
                    size_t anchor_pos = section_key.find('&');
                    if (anchor_pos != std::string::npos) {
                        current_anchor_key = section_key.substr(anchor_pos + 1);
                        current_anchor_key.erase(0, current_anchor_key.find_first_not_of(" \t"));
                        current_section = section_key.substr(0, anchor_pos);
                        current_section.erase(current_section.find_last_not_of(" \t") + 1);
                        // Clear any existing content for this anchor
                        anchors[current_anchor_key].clear();
                        std::cout << "DEBUG: Found top-level anchor definition '" << current_anchor_key << "' for section '" << current_section << "'" << std::endl;
                    } else {
                        current_section = section_key;
                        current_anchor_key = "";
                    }
                    current_subsection = "";
                }
            } else if (indent <= 2) {
                // Second level
                if (line.back() == ':') {
                    current_subsection = line.substr(0, line.length() - 1);
                    current_subsection.erase(0, current_subsection.find_first_not_of(" \t"));
                    
                    // Check if this is an anchor definition
                    size_t anchor_pos = current_subsection.find('&');
                    if (anchor_pos != std::string::npos) {
                        current_anchor_key = current_subsection.substr(anchor_pos + 1);
                        current_anchor_key.erase(0, current_anchor_key.find_first_not_of(" \t"));
                        current_subsection = current_subsection.substr(0, anchor_pos);
                        current_subsection.erase(current_subsection.find_last_not_of(" \t") + 1);
                        // Clear any existing content for this anchor
                        anchors[current_anchor_key].clear();
                    } else {
                        current_anchor_key = "";
                    }
                } else {
                    parseKeyValue(line, current_section, current_subsection, current_anchor_key);
                }
            } else {
                // Third level or list items
                parseKeyValue(line, current_section, current_subsection, current_anchor_key);
            }
        }
        
        return true;
    }
    
private:
    void parseAnchors(const std::vector<std::string>& lines) {
        std::string current_anchor = "";
        
        for (const std::string& line_raw : lines) {
            std::string line = line_raw;
            
            // Check indentation level BEFORE trimming
            size_t indent = 0;
            for (char c : line) {
                if (c == ' ') indent++;
                else break;
            }
            
            // Remove comments and trim whitespace
            size_t comment_pos = line.find('#');
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }
            
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (line.empty()) continue;
            
            
            // Look for anchor definitions
            size_t anchor_pos = line.find('&');
            if (anchor_pos != std::string::npos && line.find(':') != std::string::npos) {
                // This line contains both : and &, it's an anchor definition
                current_anchor = line.substr(anchor_pos + 1);
                current_anchor.erase(0, current_anchor.find_first_not_of(" \t"));
                current_anchor.erase(current_anchor.find_last_not_of(" \t") + 1);
                // Clear any existing content for this anchor
                anchors[current_anchor].clear();
            } else if (line.back() == ':') {
                current_anchor = "";
            }
            
            if (!current_anchor.empty() && line.front() == '-') {
                // This is a list item for the current anchor
                std::string item = line.substr(1);
                item.erase(0, item.find_first_not_of(" \t"));
                // Remove quotes if present
                if (!item.empty() && ((item.front() == '"' && item.back() == '"') || 
                    (item.front() == '\'' && item.back() == '\''))) {
                    item = item.substr(1, item.length() - 2);
                }
                anchors[current_anchor].push_back(item);
            }
        }
    }
    void parseKeyValue(const std::string& line, const std::string& section, const std::string& subsection, const std::string& current_anchor = "") {
        if (line.front() == '-') {
            // List item
            std::string item = line.substr(1);
            item.erase(0, item.find_first_not_of(" \t"));
            
            // Handle references (*anchor_name)
            if (!item.empty() && item.front() == '*') {
                std::string anchor_name = item.substr(1);
                if (anchors.count(anchor_name)) {
                    // Expand the anchor
                    std::string key = section + (subsection.empty() ? "" : "." + subsection);
                    for (const auto& anchor_item : anchors[anchor_name]) {
                        lists[key].push_back(anchor_item);
                    }
                    return;
                }
            }
            
            // Remove quotes if present
            if (!item.empty() && ((item.front() == '"' && item.back() == '"') || 
                (item.front() == '\'' && item.back() == '\''))) {
                item = item.substr(1, item.length() - 2);
            }
            
            std::string key = section + (subsection.empty() ? "" : "." + subsection);
            lists[key].push_back(item);
            
            // If we're in an anchor definition, also store in anchors
            if (!current_anchor.empty()) {
                anchors[current_anchor].push_back(item);
            }
        } else {
            // Key-value pair
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                
                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // Check for anchor definition (key: &anchor_name)
                size_t anchor_pos = key.find('&');
                std::string anchor_name = "";
                if (anchor_pos != std::string::npos) {
                    // Extract anchor name
                    anchor_name = key.substr(anchor_pos + 1);
                    anchor_name.erase(0, anchor_name.find_first_not_of(" \t"));
                    // Remove the anchor part from the key
                    key = key.substr(0, anchor_pos);
                    key.erase(key.find_last_not_of(" \t") + 1);
                }
                
                // Handle array notation [item1, item2, ...]
                if (!value.empty() && value.front() == '[' && value.back() == ']') {
                    std::string array_content = value.substr(1, value.length() - 2);
                    std::string full_key = section + (subsection.empty() ? "" : "." + subsection) + "." + key;
                    
                    std::vector<std::string> items;
                    
                    // Split by comma
                    std::stringstream ss(array_content);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        // Trim whitespace
                        item.erase(0, item.find_first_not_of(" \t"));
                        item.erase(item.find_last_not_of(" \t") + 1);
                        // Remove quotes
                        if (!item.empty() && ((item.front() == '"' && item.back() == '"') || 
                            (item.front() == '\'' && item.back() == '\''))) {
                            item = item.substr(1, item.length() - 2);
                        }
                        if (!item.empty()) {
                            items.push_back(item);
                            lists[full_key].push_back(item);
                        }
                    }
                    
                    // Store anchor if defined
                    if (!anchor_name.empty()) {
                        anchors[anchor_name].clear();
                        anchors[anchor_name] = items;
                    }
                } else {
                    // Regular key-value pair
                    // Remove quotes if present
                    if (!value.empty() && ((value.front() == '"' && value.back() == '"') || 
                        (value.front() == '\'' && value.back() == '\''))) {
                        value = value.substr(1, value.length() - 2);
                    }
                    
                    std::string full_key = section + (subsection.empty() ? "" : "." + subsection) + "." + key;
                    values[full_key] = value;
                }
            }
        }
    }
};

ConfigParser::ConfigParser() {
    SetDefaults();
}

ConfigParser::~ConfigParser() {}

void ConfigParser::SetDefaults() {
    config_.name = "default_analysis";
    config_.luminosity = 400.0;
    config_.output_json = "output.json";
    config_.output_dir = "./json/";
    config_.verbosity = 1;
    config_.parallel = false;
    config_.dry_run = false;
}

bool ConfigParser::LoadConfig(const std::string& config_file) {
    return LoadYAML(config_file);
}

bool ConfigParser::LoadYAML(const std::string& config_file) {
    SimpleYAMLParser parser;
    
    if (!parser.parse(config_file)) {
        return false;
    }
    
    // Parse analysis section
    if (parser.values.count("analysis.name")) {
        config_.name = parser.values["analysis.name"];
    }
    if (parser.values.count("analysis.luminosity")) {
        config_.luminosity = std::stod(parser.values["analysis.luminosity"]);
    }
    if (parser.values.count("analysis.output_json")) {
        config_.output_json = parser.values["analysis.output_json"];
    }
    if (parser.values.count("analysis.output_dir")) {
        config_.output_dir = parser.values["analysis.output_dir"];
    }
    
    
    
    
    // Parse samples
    if (parser.lists.count("samples.backgrounds")) {
        config_.backgrounds = parser.lists["samples.backgrounds"];
    }
    if (parser.lists.count("samples.signals")) {
        config_.signals = parser.lists["samples.signals"];
    }
    if (parser.lists.count("samples.data")) {
	config_.data = parser.lists["samples.data"];
    }
    
    // Parse bins
    for (const auto& pair : parser.lists) {
        if (pair.first.find("bins.") == 0) {
            std::string full_key = pair.first;
            std::string bin_name;
            
            // Extract bin name from the key
            size_t bins_pos = full_key.find("bins.");
            if (bins_pos != std::string::npos) {
                std::string remainder = full_key.substr(bins_pos + 5); // Remove "bins."
                size_t dot_pos = remainder.find('.');
                
                if (dot_pos != std::string::npos) {
                    // Key like "bins.single_bin.cuts" - not used in our format
                    continue;
                } else {
                    // Key like "bins.single_bin" - this is our cuts list
                    bin_name = remainder;
                }
            }
            
            if (!bin_name.empty()) {
                BinConfig bin_config;
                bin_config.name = bin_name;
                bin_config.cuts = pair.second;
                
                // Look for description
                std::string desc_key = "bins." + bin_name + ".description";
                if (parser.values.count(desc_key)) {
                    bin_config.description = parser.values[desc_key];
                }
                
                config_.bins.push_back(bin_config);
            }
        }
    }
    
    // Parse options
    if (parser.values.count("options.verbosity")) {
        config_.verbosity = std::stoi(parser.values["options.verbosity"]);
    }
    if (parser.values.count("options.parallel")) {
        config_.parallel = (parser.values["options.parallel"] == "true");
    }
    if (parser.values.count("options.dry_run")) {
        config_.dry_run = (parser.values["options.dry_run"] == "true");
    }
    
    return ValidateConfig();
}

std::string ConfigParser::GetCombinedCuts(const std::string& bin_name) const {
    for (const auto& bin : config_.bins) {
        if (bin.name == bin_name) {
            if (bin.cuts.empty()) return "";
            
            std::string combined = "(" + bin.cuts[0] + ")";
            for (size_t i = 1; i < bin.cuts.size(); ++i) {
                combined += " && (" + bin.cuts[i] + ")";
            }
            return combined;
        }
    }
    return "";
}

void ConfigParser::PrintConfig() const {
    std::cout << "=== Analysis Configuration ===" << std::endl;
    std::cout << "Name: " << config_.name << std::endl;
    std::cout << "Luminosity: " << config_.luminosity << " fb^-1" << std::endl;
    std::cout << "Output JSON: " << config_.output_json << std::endl;
    std::cout << "Output Directory: " << config_.output_dir << std::endl;
    
    std::cout << "\nBackgrounds: ";
    for (const auto& bg : config_.backgrounds) {
        std::cout << bg << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Signals: ";
    for (const auto& sig : config_.signals) {
        std::cout << sig << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Data: ";
    for (const auto& dat : config_.data) {
	std::cout << dat << " ";
    }

    std::cout << "\nAnalysis Bins:" << std::endl;
    for (const auto& bin : config_.bins) {
        std::cout << "  " << bin.name << ": " << bin.description << std::endl;
        std::cout << "    Cuts: " << GetCombinedCuts(bin.name) << std::endl;
    }
    
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  Verbosity: " << config_.verbosity << std::endl;
    std::cout << "  Parallel: " << (config_.parallel ? "true" : "false") << std::endl;
    std::cout << "  Dry run: " << (config_.dry_run ? "true" : "false") << std::endl;
}

bool ConfigParser::ValidateConfig() const {
    if (config_.luminosity <= 0) {
        std::cerr << "Error: Luminosity must be positive" << std::endl;
        return false;
    }
    
    if (config_.backgrounds.empty()) {
        std::cerr << "Warning: No background samples specified" << std::endl;
    }
    
    if (config_.bins.empty()) {
        std::cerr << "Error: No analysis bins defined" << std::endl;
        return false;
    }
    
    return true;
}
