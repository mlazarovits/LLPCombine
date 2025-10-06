
#ifndef BFTOOLS_H
#define BFTOOLS_H
#include <string>
#include <map>
#include <sstream> 
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm> // For std::all_of
#include <cctype>    // For std::isdigit

class Process{
	
	public: 
	std::string procname{};
	long long unsigned int nevents{};
	double wnevents{};
	double staterror{};

	Process(std::string name) :procname(name), nevents(0), wnevents(0), staterror(0){}
	Process(std::string name, long long unsigned int n, double wn, double err) :procname(name), nevents(n), wnevents(wn), staterror(err){}
	//assume it is initialized from 0
	void Add(Process* p){
		nevents += p->nevents;
		wnevents += p->wnevents;
		staterror += p->staterror * p->staterror;
	}
	void FixError(){
		staterror = std::sqrt(staterror);
	}
};
class Bin{
	
	public:
	std::string binname{};
	std::map<std::string, Process*> bkgProcs{};
	std::map<std::string, Process*> combinedProcs{};
	std::map<std::string, Process*> signals{};
	std::map<std::string, Process*> dataProcs{};

	std::pair<std::string, Process*> totalBkg{};
	std::pair<std::string, Process*> totalData{};


};

class BFTool{

	public:
	static std::vector<std::string> SplitString(const std::string& str,const std::string& delimiter);
	static std::string GetSignalTokens(std::string& input);
	static bool  ContainsAnySubstring(const std::string& mainString, const std::vector<std::string>& substrings);
};

inline std::vector<std::string> BFTool::SplitString(const std::string& str,const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t prev_pos = 0;
    size_t current_pos;

    while ((current_pos = str.find(delimiter, prev_pos)) != std::string::npos) {
        tokens.push_back(str.substr(prev_pos, current_pos - prev_pos));
        prev_pos = current_pos + delimiter.length();
    }
    tokens.push_back(str.substr(prev_pos)); // Add the last token

    return tokens;
}
inline std::string BFTool::GetSignalTokens(std::string& input ){
	std::string mode = "x";
	std::string mgo = "0";
	std::string mn2 = "0";
	std::string mn1 = "0";
	std::string ctau = "10";
	std::vector<std::string> siglist_toks = SplitString(input, "/");
	std::string sig = siglist_toks[ siglist_toks.size()-1];
	std::vector<std::string> sig_toks = SplitString(sig, "_");
	
	mode = sig_toks[3];
	mgo = SplitString(sig_toks[5], "-")[1];
	mn2 = SplitString(sig_toks[6], "-")[1];
	mn1 = SplitString(sig_toks[7], "-")[1];
//	ctau = SplitString(sig_toks[7], "-")[2];
//squarks have different naming atm
	ctau = sig_toks[8];

	ctau = SplitString(ctau, "p")[1];
	//fancy padding operations to deal with ctau <1 //////////////////////
	char padding_char = '0'; // Assuming '0' as padding character
	std::string padded_str = ctau;
    size_t first_digit_pos = padded_str.find_first_not_of(padding_char);
    if (first_digit_pos == std::string::npos) { // String is all padding or empty
        // Handle this case, e.g., if "000", then value is 0 and padding is 3
        first_digit_pos = padded_str.length(); // Treat as if numeric part is empty
    }

//    int padding_count = first_digit_pos;
    std::string numeric_str = padded_str.substr(first_digit_pos);
    int num_value = std::stoi(numeric_str);
    num_value *= 10; //make it ctau in cm (10 because we take the digit past the p in m (.x) 
    //end padding
	
	std::string new_numeric_str = std::to_string(num_value);

    // Determine target total length (original string length)
    size_t target_length = padded_str.length();

    // Calculate new padding count
    int new_padding_count = 0;
    if (target_length > new_numeric_str.length()) {
        new_padding_count = target_length - new_numeric_str.length();
    }


    std::string final_padded_str = std::string(new_padding_count, padding_char) + new_numeric_str;
    ////end padding
	
	ctau = final_padded_str;
	
	std::string signalKeys = mode+"_"+mgo+"_"+mn2+"_"+mn1+"_"+ctau;
	return signalKeys;
	
	
}
inline bool BFTool::ContainsAnySubstring(const std::string& mainString, const std::vector<std::string>& substrings) {
    for (const std::string& sub : substrings) {
	//std::cout<<"comparing string: "<< mainString <<", "<<sub<<"\n";
        if (mainString.find(sub) != std::string::npos) {
            // Substring found
            return true; 
        }
    }
    // No substring found
    return false; 
}
#endif
