
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
#include <cmath>
#include <stdexcept>
#include "Math/ProbFunc.h"
#include "Math/PdfFuncMathCore.h"


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
	std::pair<std::string, Process*> totalBkg{};
	std::pair<std::string, Process*> data{};

};

class BFTool{

		public:
		static std::vector<std::string> SplitString(const std::string& str,const std::string& delimiter);
		static std::string GetSignalTokens(std::string& input);
		static std::string NormalizeCtauToken(const std::string& ctau_token);
		static bool  ContainsAnySubstring(const std::string& mainString, const std::vector<std::string>& substrings);
		static std::string RoundNumber(const std::string& str, int ndigits);
};
inline std::string BFTool::RoundNumber(const std::string& str, int ndigits = 1){
	std::string ret;
	if(str[str.find(".")+1] == '0')
		ret = str.substr(0,str.find("."));
	else{
		ret = str.substr(0,str.find(".")+1+ndigits);
	}
	return ret;
}

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
	inline std::string BFTool::NormalizeCtauToken(const std::string& ctau_token){
		std::string ctau = ctau_token;
		if(ctau.rfind("ct", 0) == 0)
			ctau = ctau.substr(2);

		if(ctau.empty())
			throw std::invalid_argument("Could not parse empty ctau token: " + ctau_token);

		// Preserve the historical process-key conversion used by existing SMS files:
		// ct0p1 -> 10, ct0p5 -> 50, etc.
		if(ctau.rfind("0p", 0) == 0){
			std::vector<std::string> toks = SplitString(ctau, "p");
			if(toks.size() < 2)
				throw std::invalid_argument("Could not parse ctau token: " + ctau_token);

			char padding_char = '0';
			std::string padded_str = toks[1];
			size_t first_digit_pos = padded_str.find_first_not_of(padding_char);
			if (first_digit_pos == std::string::npos)
				return "0";

			std::string numeric_str = padded_str.substr(first_digit_pos);
			int num_value = std::stoi(numeric_str);
			num_value *= 10;

			std::string new_numeric_str = std::to_string(num_value);
			size_t target_length = padded_str.length();
			int new_padding_count = 0;
			if (target_length > new_numeric_str.length())
				new_padding_count = target_length - new_numeric_str.length();

			return std::string(new_padding_count, padding_char) + new_numeric_str;
		}

		// Newer files may spell meter values with an extra separator, e.g. ct-3p0.
		// Treat ct-3p0 like ct3p0: 3.0 m -> 300 cm.
		if(ctau[0] == '-')
			ctau = ctau.substr(1);

		if(ctau.find('p') != std::string::npos){
			std::replace(ctau.begin(), ctau.end(), 'p', '.');
			double meters = std::stod(ctau);
			long long cm = std::llround(meters * 100.0);
			return std::to_string(cm);
		}

		if(!std::all_of(ctau.begin(), ctau.end(), [](unsigned char c){ return std::isdigit(c); }))
			throw std::invalid_argument("Could not parse ctau token: " + ctau_token);

		return ctau;
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
		
		for(const auto& tok : sig_toks){
			if(tok == "gogoGZ" || tok == "gogoG" || tok == "gogoZ" || tok == "sqsqG")
				mode = tok;
			else if(tok.rfind("mGl-", 0) == 0)
				mgo = SplitString(tok, "-")[1];
			else if(tok.rfind("mN2-", 0) == 0)
				mn2 = SplitString(tok, "-")[1];
			else if(tok.rfind("mN1-", 0) == 0)
				mn1 = SplitString(tok, "-")[1];
			else if(tok.rfind("ct", 0) == 0)
				ctau = NormalizeCtauToken(tok);
		}

		if(mode == "x" || mgo == "0" || mn2 == "0" || mn1 == "0")
			throw std::invalid_argument("Could not parse signal filename: " + sig);
		
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
class ReweightTool{

    public:
    static float LifetimeReweight(float ctau, float tau_old, float tau_new);
	static double DecayReweight( bool isZZ, bool isGZ, bool isGG, double Z_old, double Z_new, double G_old, double G_new );    
};
inline float ReweightTool::LifetimeReweight(float ctau, float tau_old, float tau_new){

    double inv_old = 1.0 / tau_old;
    double inv_new = 1.0 / tau_new;
    return float( (tau_old / tau_new) * std::exp(ctau * (inv_old - inv_new)) );
}
inline double ReweightTool::DecayReweight( bool isZZ, bool isGZ, bool isGG, double Z_old, double Z_new, double G_old, double G_new ){

        //compute original probabilities
		//old and new should never be 0 or 1 other wise we cant reweight! so we dont need -nan protection
        double pZZ0 = ROOT::Math::binomial_pdf(2, Z_old, 2);	
        double pGG0 = ROOT::Math::binomial_pdf(2, G_old, 2);
        double pGZ0 = 1. - (pZZ0+pGG0);


        //compute new probs
		double pZZ1 = -1;
		if(Z_new == 0 || Z_new == 1){
       		pZZ1 = Z_new;
		}else{
			pZZ1 = ROOT::Math::binomial_pdf(2, Z_new, 2);
		}
		double pGG1 = -1;
		if(G_new == 0 || G_new == 1){
			pGG1 = G_new;
		}else{	
			pGG1 = ROOT::Math::binomial_pdf(2, G_new, 2);
		}
        double pGZ1 = 1. - (pZZ1+pGG1);

        double decaywt=0.;
        //get the event weight
        if(isZZ){
               	decaywt = pZZ1/pZZ0;
        }
        if(isGZ){
                decaywt = pGZ1/pGZ0;
        }
        if(isGG){
                decaywt = pGG1/pGG0;
        }
        return decaywt;
}

#endif
