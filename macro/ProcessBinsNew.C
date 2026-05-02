#include <string>
#include <vector>
using std::string;
using std::vector;

void FillBin( TH1D* combinedhist,int num, std::string binN, TFile* f, std::string bindir){
	//bindir = shapes_prefit or shapes_fit_b
	std::string targetPath = bindir+"/"+binN+"/total_background";
	TH1D* h = (TH1D*) f->Get(targetPath.c_str());
	std::cout<<"accessing "<<targetPath;
	std::cout <<" with bin content " << h->GetBinContent(1) << " and error " << h->GetBinError(1) << endl;
	combinedhist->SetBinContent(num, h->GetBinContent(1));
	combinedhist->SetBinError(num, h->GetBinError(1));
	//std::cout<<h->GetBinContent(1)<<"\n";
}
void FillDataBin( TH1D* combinedData,int num, std::string binN, TFile* f){
	std::string targetPath = "shapes_prefit/"+binN+"/data";
	TGraphAsymmErrors* h = (TGraphAsymmErrors*) f->Get(targetPath.c_str());
	double x;
	double y;
	h->GetPoint(0,x,y);
	//std::cout<<"accessing "<<targetPath<<" with bin content " << y << " and error " << sqrt(y) << endl;
	combinedData->SetBinContent(num, y);
	combinedData->SetBinError(num, std::sqrt(y));
	
}

void ProcessBinsNew(string f, string workdir = "./", string oname = "test"){

	//std::string f = "fitDiagnosticsSVOnly4BinCR.root";
	//std::vector<string> chs = {"Ch1CRPho1PromptMedIso", "Ch2CRPho2PromptMedIso", "Ch3CRSVHadLowDxy", "Ch4CRSVHadHighDxy"};
	//std::vector<string> chs = {"Ch1CRPho1PromptMedIso", "Ch2CRPho2PromptMedIso"};//, "Ch3CRSVHad", "Ch4CRSVHad"};
	//std::vector<string> chs = {"Ch3CRSVHadLowDxy", "Ch4CRSVHadHighDxy"};

	//std::vector<string> chs = {"Ch1CRgeq1PhoBHEarlyBin","Ch2CRgeq1PhoBHLateBin","Ch3CRgeq1PhoNotBHEarlyBin","Ch4CRgeq1PhoNotBHLateBin"};
	//std::vector<string> chs = {"Ch1CRPhoBHEarly","Ch2CRPhoBHLate","Ch3CRPhonotBHEarly","Ch4CRPhonotBHLate","Ch5CRPho1Iso","Ch6CRPho2Iso","Ch7CRHadLow","Ch8CRHadHigh"};
	//std::vector<string> chs = {"Ch5CRgeq1PhoMedIsoPromptBin","Ch6SReq1PhoTightIsoPromptBin", "Ch6SReq2PhoTightIsoPromptBin"};
	//std::vector<string> chs = {"Ch5CRgeq1PhoMedIsoPromptBin","Ch6SReq2PhoTightIsoPromptBin"};
	std::vector<string> chs = {"Ch5CReq1PhoMedIsoPromptBin","Ch6SReq1PhoTightIsoPromptBin","Ch5CReq2PhoMedIsoPromptBin","Ch6SReq2PhoTightIsoPromptBin"};

	TFile* tf = TFile::Open(f.c_str());
	
	//binorders
	string anchorbin = "00";
	//bingrid = {"00"};
	
	vector<string> chs2bin = {"Ch1","Ch2","Ch3","Ch4"};
	vector<string> chs4bin = {"Ch5","Ch6","Ch7","Ch8"};

	vector<TH1D*> hists;
	std::vector<std::string> bingrid;
	for(auto ch : chs){
		cout << "doing channel " << ch << endl;
		bingrid = {"00","10","01","11"};
		TH1D* prefitch1= new TH1D(("hpre"+ch).c_str(), "prefit shape", bingrid.size(), -0.5,bingrid.size()+0.5);
		TH1D* bpostfitch1= new TH1D(("hpostb"+ch).c_str(), "post-fit bonly",bingrid.size(),-0.5,bingrid.size()+0.5);
		TH1D* hdatach1 = new TH1D(("hdata"+ch).c_str(), "data",bingrid.size(),-0.5,bingrid.size()+0.5);
		tf->cd();
		//combine bins into 1 plot
		for(int i=0; i<bingrid.size(); i++){
			string chbin = ch+bingrid[i];
			if(anchorbin != "" && bingrid[i] == anchorbin && ch.find("SR") != string::npos){
				chbin = chbin.replace(chbin.find("SR"),2,"CR");
			}
			FillBin( prefitch1,i+1,    chbin, tf, "shapes_prefit");
			FillBin( bpostfitch1,i+1,  chbin, tf, "shapes_fit_b");
			FillDataBin( hdatach1,i+1, chbin, tf);
			//break;
		}
		hists.push_back(prefitch1);
		hists.push_back(bpostfitch1);
		hists.push_back(hdatach1);
	}
	TFile* fout = new TFile((workdir+oname+"combinedBinHists.root").c_str(), "RECREATE");
	cout << "writing processed hists to " << fout->GetName() << endl;
	fout->cd();
	for(auto hist : hists)
		hist->Write();
	tf->cd();	
	tf->Close();
	fout->cd();
	fout->Close();


}
