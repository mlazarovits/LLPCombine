



void MakePlot(std::string dir, std::string ch, std::string title){
	std::string targetPath = dir+"/combinedBinHists.root";
	TFile* f = TFile::Open(targetPath.c_str());
	TH1D* hpre = (TH1D*) f->Get(("hpre"+ch).c_str());
	TH1D* hpost = (TH1D*) f->Get(("hpostb"+ch).c_str());
	TH1D* hdata = (TH1D*) f->Get(("hdata"+ch).c_str());
	
	hpre->SetLineColor(kBlue);
	hpre->SetLineWidth(2);
	//hpre->SetMarkerStyle(8);
	
	hpost->SetLineColor(kMagenta);
	hpost->SetLineWidth(2);
	//hpost->SetMarkerStyle
	
	int numBins = hdata->GetNbinsX();
	TGraph *graph = new TGraph(numBins);
	for (int i = 1; i <= numBins; ++i) {
    double x = hdata->GetBinCenter(i); // Get the center of the bin for the x-coordinate
    double y = hdata->GetBinContent(i); // Get the bin content for the y-coordinate
    graph->SetPoint(i-1, x, y); // Set the point in the TGraph
	}
	graph->SetMarkerStyle(8); // Set marker style
	graph->SetMarkerColor(kBlack); // Blue markers
	
	//std::vector<std::string> bingrid = {"00","01","02","10","11","12","20","21","22"};
	std::vector<std::string> bingrid = {"00","10","20","01","11","21","02","12","22"};
	for( int i=0; i<bingrid.size(); i++){
		hpre->GetXaxis()->SetBinLabel(i+1, bingrid[i].c_str());
	}


	hpre->SetTitle(title.c_str());
	hpre->Draw();
	hpost->Draw("SAME");
	graph->Draw("P SAME");
	
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
	MakePlot("Datafit", "CRHad", "Data 9 bin fit");
	//MakePlot("Datafit", "CRLep", "Data 9 bin fit");
	
}
