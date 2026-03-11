#include <string>
#include <vector>
using std::string;
using std::vector;

void FillBin( TH1D* combinedhist,int num, std::string binN, TFile* f, std::string bindir){
	//bindir = shapes_prefit or shapes_fit_b
	std::string targetPath = bindir+"/"+binN+"/total_background";
	TH1D* h = (TH1D*) f->Get(targetPath.c_str());
	std::cout<<"accessing "<<targetPath<<" bin content " << h->GetBinContent(1) << " error " << h->GetBinError(1) << std::endl;
	combinedhist->SetBinContent(num, h->GetBinContent(1));
	combinedhist->SetBinError(num, h->GetBinError(1));
	std::cout<<h->GetBinContent(1)<<"\n";
}
void FillDataBin( TH1D* combinedData,int num, std::string binN, TFile* f){
	std::string targetPath = "shapes_prefit/"+binN+"/data";
	TGraphAsymmErrors* h = (TGraphAsymmErrors*) f->Get(targetPath.c_str());
	double x;
	double y;
	h->GetPoint(0,x,y);
	std::cout<<"accessing "<<targetPath<<" bin content " << y << " err " << sqrt(y) << std::endl;
	combinedData->SetBinContent(num, y);
	combinedData->SetBinError(num, std::sqrt(y));
	
}

void ProcessABCDChannel(){

	std::string workdir = "./";
	std::string f = "fitDiagnostics1PhoDelayed.root";
	std::vector<string> chs = {"Ch1CR1PhoBHEarly","Ch2CR1PhoBHLate","Ch3CR1PhonotBHEarly","Ch4SR1PhonotBHLate"};
	string extra = "Pho1Delayed";
	TH1D* prefitch1= new TH1D(("hpre"+extra).c_str(), "prefit shape", chs.size(), -0.5,chs.size()+0.5);
	TH1D* bpostfitch1= new TH1D(("hpostb"+extra).c_str(), "post-fit bonly",chs.size(),-0.5,chs.size()+0.5);
	TH1D* hdatach1 = new TH1D(("hdata"+extra).c_str(), "data",chs.size(),-0.5,chs.size()+0.5);
	TFile* tf = TFile::Open(f.c_str());
	
	//binorders
	std::vector<std::string> bingrid = {"00"};//,"10","01","11"};
	vector<TH1D*> hists;
	for(int ch = 0; ch < chs.size(); ch++){
		cout << "doing channel " << ch << endl;
		tf->cd();

		//combine channels into 1 plot
		//only 1 bin per channel for now
		for(int i = 0; i < bingrid.size(); i++){
			FillBin( prefitch1,   ch+1, chs[ch]+bingrid[i], tf, "shapes_prefit");
			FillBin( bpostfitch1, ch+1, chs[ch]+bingrid[i], tf, "shapes_fit_b");
			FillDataBin( hdatach1,ch+1, chs[ch]+bingrid[i], tf);
			prefitch1->GetXaxis()->SetBinLabel(ch+1,(chs[ch]+bingrid[i]).c_str());
			bpostfitch1->GetXaxis()->SetBinLabel(ch+1,(chs[ch]+bingrid[i]).c_str());
			hdatach1->GetXaxis()->SetBinLabel(ch+1,(chs[ch]+bingrid[i]).c_str());
		}
	}
	hists.push_back(prefitch1);
	hists.push_back(bpostfitch1);
	hists.push_back(hdatach1);



	TFile* fout = new TFile((workdir+"1PhoDelayedcombinedBinHists.root").c_str(), "RECREATE");
	fout->cd();
	for(auto hist : hists)
		hist->Write();
	tf->cd();	
	tf->Close();
	fout->cd();
	fout->Close();


}
