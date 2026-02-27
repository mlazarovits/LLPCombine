#include <string>
#include <vector>
#include <TStyle.h>
using std::vector;
using std::string;

void GetBinGrid(int nbins, vector<string>& bingrid){
	bingrid.clear();
	//assume grid is nxn where n = sqrt(nbins)
	for(int y = 0; y < (int)sqrt(nbins); y++){
		string ybin = std::to_string(y);
		for(int x = 0; x < (int)sqrt(nbins); x++){
			string xbin = std::to_string(x);
			bingrid.push_back( xbin+ybin );
		}
	}
}


TCanvas* MakePlot(std::string targetPath, std::string ch, std::string title){
	TCanvas* cv = new TCanvas(title.c_str(), title.c_str());
	TFile* f = TFile::Open(targetPath.c_str());
	TH1D* hpre = (TH1D*) f->Get(("hpre"+ch).c_str());
	TH1D* hpost = (TH1D*) f->Get(("hpostb"+ch).c_str());
	TH1D* hdata = (TH1D*) f->Get(("hdata"+ch).c_str());
	TLegend* leg = new TLegend(0.7,0.7,0.8,0.9);
	gStyle->SetOptStat(0000);

	hpre->SetLineColor(kBlue);
	hpre->SetLineWidth(2);
	//hpre->SetMarkerStyle(8);
	leg->AddEntry(hpre,"Prefit","l");

	
	hpost->SetLineColor(kMagenta);
	hpost->SetLineWidth(2);
	//hpost->SetMarkerStyle
	leg->AddEntry(hpost,"Postfit","l");
cout << "ch" << ch << endl;	
	double maxy = 0;
	int numBins = hdata->GetNbinsX();
	TGraph *graph = new TGraph(numBins);
	for (int i = 1; i <= numBins; ++i) {
    double x = hdata->GetBinCenter(i); // Get the center of the bin for the x-coordinate
    double y = hdata->GetBinContent(i); // Get the bin content for the y-coordinate
    graph->SetPoint(i-1, x, y); // Set the point in the TGraph
    		if(y > maxy)
			maxy = y;
	}
	graph->SetMarkerStyle(8); // Set marker style
	graph->SetMarkerColor(kBlack); // Blue markers
	leg->AddEntry(graph,"data","p");

	//std::vector<std::string> bingrid = {"00","01","02","10","11","12","20","21","22"};
	//std::vector<std::string> bingrid = {"00","10","20","01","11","21","02","12","22"};
	vector<string> bingrid;
	GetBinGrid(hpre->GetNbinsX(), bingrid);

	for( int i=0; i<bingrid.size(); i++){
		//hpre->GetXaxis()->SetBinLabel(i+1, bingrid[i].c_str());
		//hpre->GetXaxis()->SetBinLabel(i+1, );
		if(hpre->GetBinContent(i+1) > maxy)
			maxy = hpre->GetBinContent(i+1);
		if(hpost->GetBinContent(i+1) > maxy)
			maxy = hpost->GetBinContent(i+1);
	}


	hpre->SetTitle(title.c_str());
	hpre->GetYaxis()->SetRangeUser(0,maxy+0.2*maxy);
	cv->cd();
	hpre->Draw();
	hpost->Draw("SAME");
	graph->Draw("P SAME");
	leg->Draw("same");
	return cv;	
}

void Plot9Shapes(){
	//automcfit  automconlyfit  basefit  bbbfit  MClnN  mclnNupdn

	//MakePlot("basefit","Base shape as lnNs, uncertainty from up/down shape");
	//MakePlot("bbbfit","shape as lnNs + bbb uncertainty");
	//MakePlot("automcfit", "shape as lnNs with auto MC stats");
	//MakePlot("MClnN", "shape as lnNs with lnN  only Stat uncertainty");
	//MakePlot("mclnNupdn", "shape as lnNs with lnN Stat+Shape uncertainty");
	//MakePlot("MCfit", "CRHad", "MC 9 bin fit");
	//MakePlot("MCfit", "CRLep", "MC 9 bin fit");
	vector<TCanvas*> cvs;
	//vector<string> chs = {"Ch1CRPho1PromptMedIso","Ch2CRPho2PromptMedIso","Ch3CRSVHad","Ch4CRSVHad"};
	//vector<string> chs = {"Ch1CRPho1PromptMedIso","Ch2CRPho2PromptMedIso"};
	//vector<string> chs = {"Ch3CRSVHadLowDxy","Ch4CRSVHadHighDxy"};

	vector<string> chs = {"Pho1Delayed"};//{"Ch1CR1PhoBHEarly","Ch2CR1PhoBHLate","Ch3CR1PhonotBHEarly","Ch4SR1PhonotBHLate"};
	for(auto ch : chs){
		TCanvas* cv = MakePlot("1PhoDelayedcombinedBinHists.root", ch, ch+"_Data_4binfit");
		cvs.push_back(cv);
	}
	TFile* fout = new TFile("1phodelayedshapes.root","RECREATE");
	fout->cd();
	for(auto cv : cvs)
		cv->Write();
	fout->Close();	
}
