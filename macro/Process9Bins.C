
void FillBin( TH1D* combinedhist,int num, std::string binN, TFile* f, std::string bindir){
	//bindir = shapes_prefit or shapes_fit_b
	std::string targetPath = bindir+"/"+binN+"/total_background";
	std::cout<<"accessing "<<targetPath<<" ";
	TH1D* h = (TH1D*) f->Get(targetPath.c_str());
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
	combinedData->SetBinContent(num, y);
	combinedData->SetBinError(num, std::sqrt(y));
	
}

void Process9Bins(){

	//input file
//	std::string workdir = "basefit/";
//	std::string f = "basefit/fitDiagnostics.baseFitPseudoShape.root";

//	std::string workdir = "bbbfit/";
//	std::string f = workdir+"fitDiagnostics.bbbFitPseudoShape.root";
	
//	std::string workdir = "automcfit/";
//	std:string f = workdir+"fitDiagnostics.autoMCFitPseudoShape.root";

//	std::string workdir = "automconlyfit/";
//	std::string f = workdir+"fitDiagnostics.MConlyFitPseudoShape.root";

	//std::string workdir = "MClnN/";
	//std::string f = workdir+"fitDiagnostics.MClnNFitPseudoShape.root";
	
//	std::string workdir = "mclnNupdn/";
//	std::string f = workdir+"fitDiagnostics.MClnUpDnNFitPseudoShape.root";

	//std::string workdir = "MCfit/";
	//std::string f = "fitDiagnostics.MC9bin.root";

	std::string workdir = "Datafit/";
	std::string f = "fitDiagnostics.Data9bin.root";

	TFile* tf = TFile::Open(f.c_str());
	
	//binorders
	std::vector<std::string> bingrid = {"00","10","20","01","11","21","02","12","22"};
	
	std::string ch = "CRHad";
	TFile* fout = new TFile((workdir+"combinedBinHists.root").c_str(), "RECREATE");
	
	TH1D* prefitch1= new TH1D(("hpre"+ch).c_str(), "prefit shape", 9, -0.5,9.5);
	TH1D* bpostfitch1= new TH1D(("hpostb"+ch).c_str(), "post-fit bonly",9,-0.5,9.5);
	TH1D* hdatach1 = new TH1D(("hdata"+ch).c_str(), "data",9,-0.5,9.5);
	//combine bins into 1 plot
	for(int i=0; i<bingrid.size(); i++){
		FillBin( prefitch1,i+1, ch+bingrid[i], tf, "shapes_prefit");
		FillBin( bpostfitch1,i+1, ch+bingrid[i], tf, "shapes_fit_b");
		FillDataBin( hdatach1,i+1, ch+bingrid[i], tf);
		//break;
	}
	ch = "CRLep";
	TH1D* prefitch2= new TH1D(("hpre"+ch).c_str(), "prefit shape", 9, -0.5,9.5);
	TH1D* bpostfitch2= new TH1D(("hpostb"+ch).c_str(), "post-fit bonly",9,-0.5,9.5);
	TH1D* hdatach2 = new TH1D(("hdata"+ch).c_str(), "data",9,-0.5,9.5);
	//combine bins into 1 plot
	for(int i=0; i<bingrid.size(); i++){
		FillBin( prefitch2,i+1, ch+bingrid[i], tf, "shapes_prefit");
		FillBin( bpostfitch2,i+1, ch+bingrid[i], tf, "shapes_fit_b");
		FillDataBin( hdatach2,i+1, ch+bingrid[i], tf);
		//break;
	}
	
	
	tf->Close();
	fout->Write();
	fout->Close();


}
