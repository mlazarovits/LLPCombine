#ifndef BFI_H
#define BFI_H
#include <ROOT/RDataFrame.hxx>
#include <map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "BuildFitTools.h"//Bin and process sourcej

using namespace std;
using ROOT::RDF::RNode;

typedef ROOT::RDataFrame RDF;
typedef ROOT::RDF::RNode RN;
typedef std::vector<std::string> stringlist;
typedef std::pair<std::string,std::string> proc_cut_pair;
typedef std::map< proc_cut_pair, ROOT::RDF::RResultPtr<long long unsigned int> > countmap;
typedef std::map< proc_cut_pair, ROOT::RDF::RResultPtr<double> > summap;
typedef std::map< proc_cut_pair, double> errormap;
typedef std::map< proc_cut_pair, std::unique_ptr<RN> > nodemap;


using namespace std;
using ROOT::RDF::RNode;


class BuildFitInput{
	
	public:
	map< std::string, std::unique_ptr<RNode> > _base_rdf_BkgDict{};//register actions on this guy, but build filters etc on operation/filter branching
	map< std::string, std::unique_ptr<RNode> > rdf_BkgDict{};//add branches and update this guy
	
	//same structures for signal, to be built in parallel
	map< std::string, std::unique_ptr<RNode> > _base_rdf_SigDict{};
	map< std::string, std::unique_ptr<RNode> > rdf_SigDict{};
	
	//build same for data...
	map< std::string, std::unique_ptr<RNode> > _base_rdf_DataDict{};
	map< std::string, std::unique_ptr<RNode> > rdf_DataDict{};
	
	//also keep the evtwts for error propagation later
	map< std::string, double > bkg_evtwt{};
	map< std::string, double > data_evtwt{};
	map< std::string, double > sig_evtwt{};
	
	//nodemap := (sig/bkg keyname, cut region keyname), resultptr
	nodemap bkg_filtered_dataframes;//these are the analysis bins constructed from filters
	nodemap sig_filtered_dataframes;
	nodemap data_filtered_dataframes;

	BuildFitInput();
	void BuildRVBranch();//old skims dont have rv, need to build - this needs fixed upstream
	
	//load helpers
	void LoadBkg_KeyValue( std::string key, stringlist bkglist, double Lumi );
	void LoadData_KeyValue( std::string key, stringlist bkglist, double Lumi );
	void LoadSig_KeyValue( std::string key, stringlist siglist, double Lumi );
	
	void LoadSig_byMap( map< std::string, stringlist>& SigDict, double Lumi );
	void LoadBkg_byMap( map< std::string, stringlist>& BkgDict, double Lumi );
	void LoadData_byMap( map< std::string, stringlist>& BkgDict, double Lumi );
	
	void FilterRegions( std::string filterName, std::string filterCuts );
	countmap CountRegions(nodemap& filtered_df);
	summap SumRegions(std::string branchname, nodemap& filtered_df);
	errormap ComputeStatError( countmap countResults, map< std::string, double >& evtwt );
	
	//bin objects
	std::map<std::string, Bin*> analysisbins{};
	
	//helpers print and debug df datastructures
	void ReportRegions(int verbosity=1);//report on base frame, initates action
	void PrintCountReports( countmap resultmap);
	void PrintSumReports( summap sumResults);
	void FullReport( countmap countResults, summap sumResults, errormap errorResults );
	
	//helpers bin objects
	void CreateBin(std::string binname);
	std::map<std::string, Process*> CombineBkgs( std::map<std::string, Process*>& bkgProcs );
	void ConstructBkgBinObjects( countmap countResults, summap sumResults, errormap errorResults );
	void AddSigToBinObjects( countmap countResults, summap sumResults, errormap errorResults, std::map<std::string, Bin*>& analysisbins);
	void AddDataToBinObjects( countmap countResults, summap sumResults, errormap errorResults, std::map<std::string, Bin*>& analysisbins);
	void PrintBins(int verbosity=1);
};
#endif
