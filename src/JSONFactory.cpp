#include "JSONFactory.h"


JSONFactory::JSONFactory(std::map<std::string, Bin*> analysisbins){
	//loop and add bins 
	for(const auto& it: analysisbins ){
		std::string binname = it.first;
		//std::map<std::string, Process* > bkgprocs = it.second->bkgProcs;
		std::map<std::string, Process* > combinedprocs = it.second->combinedProcs;
		std::map<std::string, Process* > signals = it.second->signals;
		std::pair<std::string, Process* > data = it.second->data;
		for(const auto& it2: combinedprocs ){
			std::string procname = it2.first;
			j[binname][procname] = { it2.second->nevents, it2.second->wnevents, it2.second->staterror };
		}
		for(const auto& it2: signals){
			std::string procname = it2.first;
			j[binname][procname] = { it2.second->nevents, it2.second->wnevents, it2.second->staterror };
		}
		//data - if specified
		if(data.second != nullptr){
			std::string procname = data.first;
			j[binname][procname] = { data.second->nevents, data.second->wnevents, data.second->staterror };
		}
	}
}
JSONFactory::JSONFactory(std::string filename){
	std::ifstream ifs(filename);
	j = json::parse(ifs);

}
std::vector<std::string> JSONFactory::GetSigProcs(){
        std::vector<std::string> sigprocs{};

        for (json::iterator it = j.begin(); it != j.end(); ++it){
                //inner loop process iterator
                std::string binname = it.key();
                for (json::iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2){
                //      std::cout<< it2.key()<<"\n";
                        if( BFTool::ContainsAnySubstring( it2.key(), sigkeys)){
                                sigprocs.push_back(it2.key());
                        }
                }
        }
        return sigprocs;
}

void JSONFactory::WriteJSON(std::string filename){
	std::cout<<"Writing json "<<filename<<" ... \n";
	std::ofstream outputFile(filename);
	if (outputFile.is_open()) {
    	outputFile << j.dump(4); // Writes with 4-space indentation
        // or outputFile << jsonData; // Writes in a compact format
        outputFile.close();
    } else {
        // Handle error if file cannot be opened
        std::cerr << "Error: Could not open file for writing." << std::endl;
    }
	    
}
