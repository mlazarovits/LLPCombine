#ifndef SAMPLETOOL_H
#define SAMPLETOOL_H
#include <map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include "BuildFitTools.h"

using namespace std;
typedef std::vector<std::string> stringlist;

class SampleTool{

	public:
	map <string, stringlist> BkgDict{};
	map <string, stringlist> SigDict{};
	map <string, stringlist> DataDict{};
	map <string, stringlist> MasterDict{};

	stringlist SignalKeys{};
	
	
	void LoadBkgs( stringlist& bkglist );
	void LoadData( stringlist& datalist );
	void LoadSigs( stringlist& siglist );
	
	SampleTool();
	
	void PrintDict( map<string, stringlist>& d );
	void PrintKeys( stringlist sl );

};

#endif
