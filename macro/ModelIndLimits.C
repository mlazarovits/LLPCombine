#include <iostream>
#include <string>
#include <TFile.h>
#include <TRandom.h>
#include <TLine.h>

using std::string;
using std::endl;
using std::cout;

string dbl2str(double indbl, int dec = 2){
	std::ostringstream out;
	out << std::fixed << std::setprecision(dec) << indbl;
	string s = out.str();
	//replace '.' with '_'
	s = s.replace(s.find("."),1,"_");
	return s;
}

bool DoCLsProcedure(double nbkg, double sigma_nbkg, string srbin, int ntoys, double lambda_sig, double obs, TH1D& bkgOnly_events, TH1D& sPlusb_events){


	TRandom3 rng;
	//create histogram for b-only distribution of events
	string bkghistname = "bkgOnly_events_lambda"+dbl2str(lambda_sig,1);
	string sighistname = "sPlusb_events_lambda"+dbl2str(lambda_sig,1);
	int nbins = int(nbkg + 50*sigma_nbkg);
	bkgOnly_events = TH1D(bkghistname.c_str(), bkghistname.c_str(),nbins,0,nbins);
	//create histogram for s+b distribution of events
	sPlusb_events = TH1D(sighistname.c_str(),sighistname.c_str(),nbins,0,nbins);
	for(int i = 0; i < ntoys; i++){
		//cout << "itoy " << i << endl;
		//sample poisson rate from log-normal parameterized by post-fit values
		//transform normal mu, sigma into lognormal_mu, lognormal_sigma for lognormal
		double lognorm_mu = log( (nbkg*nbkg) / sqrt(nbkg*nbkg + sigma_nbkg*sigma_nbkg) );
		double lognorm_sig2 = log(1 + (sigma_nbkg*sigma_nbkg)/(nbkg*nbkg));
		//if X ~ N(mu, sigma^2), then exp(X) ~ logNormal(mu, sigma^2) for expected background rate
		double lambda_bkg = exp(rng.Gaus(lognorm_mu, lognorm_sig2));
		//cout << " expected bkg rate " << lambda_bkg << " ln(rate) " << log(lambda_bkg) << endl;
		//sample from pois(lambda_bkg) for expected # events
		double bkg_nevts = rng.Poisson(lambda_bkg);
		//cout << " fill bkgOnly_event with " << bkg_nevts << endl;
		bkgOnly_events.Fill(bkg_nevts);

		//sample Nsig from poisson with varying expected rate
		double sig_nevts = rng.Poisson(lambda_sig);
		//add to # bkg events
		double tot_nevts = sig_nevts + bkg_nevts;
		//cout << " fill sPlusb_events with " << tot_nevts << " with sig_nevts = " << sig_nevts << endl;
		sPlusb_events.Fill(tot_nevts);
	}
	//cout << "bkgOnly_events integral " << bkgOnly_events.Integral() << " nentries " << bkgOnly_events.GetEntries() << " sPlusb_events " << sPlusb_events.Integral() << endl;
	//normalize histograms to make PDF (should be 1/ntoys for both)
	if(bkgOnly_events.Integral() > 0) bkgOnly_events.Scale(1/bkgOnly_events.Integral());
	if(sPlusb_events.Integral() > 0) sPlusb_events.Scale(1/sPlusb_events.Integral());

	//get p-values (integrals) from observed values
	int sPlusBbin = sPlusb_events.FindBin(obs);
	double CLsb = sPlusb_events.Integral(0,sPlusBbin);

	int bkgOnlybin = bkgOnly_events.FindBin(obs);
	double CLb = bkgOnly_events.Integral(0,bkgOnlybin);
cout << "CLsb " << CLsb << " CLb " << CLb << " CLs " << CLsb / CLb << endl;
	bool ul_found = false;
	if(CLsb / CLb <= 0.05){
		cout << "Upper limit found!! CLs = " << CLsb / CLb << " Nsig events: " << lambda_sig << endl;
		ul_found = true;
	}
	return ul_found;
}


//TODO - make cmd line args where inputs are infile and srbin (ie Ch12)
//make dictionary of ChXX to SR bin
void ModelIndLimits(){
	int ntoys = 1000;

	string srbin = "Ch12SReq2PhoTightIsoPromptBin11"; //take highest kinematic bin as SR bin for each SR
	string infilename = "FitDiagnostics/fitDiagnostics_BigGuy_NonCompressed_FullRegions_SplitSVDelayedPhoton_4BinDelayedPhoton.root";
	TFile* infile = TFile::Open(infilename.c_str(),"READ");
	

	//get bkg-only post-fit Nbkg and sigma(Nbkg) from fit diagnostics
	TH1D* bonly_postfit_bkg = (TH1D*)infile->Get(("shapes_fit_b/"+srbin+"/total_background").c_str());
	double nbkg = bonly_postfit_bkg->GetBinContent(1);
	double sigma_nbkg = bonly_postfit_bkg->GetBinError(1);
	//get observation
	double x, obs;
	TGraphAsymmErrors* data = (TGraphAsymmErrors*)infile->Get(("shapes_fit_b/"+srbin+"/data").c_str());
	data->GetPoint(0, x, obs);
	cout << "bin " << srbin << " Nbkg " << nbkg << " sigma_nbkg " << sigma_nbkg << " obs " << obs << endl;

	double d_lsig = 0.1;
	double min_lsig = double(int(nbkg));
	int nsteps = 50;
	double max_lsig = min_lsig + nsteps*d_lsig;

	string ofilename = srbin+"_CLs.root";
	TFile* ofile = TFile::Open(ofilename.c_str(),"RECREATE");
	TH1D bkgOnly_events, sPlusb_events;
	for(double lsig = min_lsig; lsig < max_lsig; lsig += d_lsig){
		cout << " lambda_sig " << lsig << endl;
		bool ul_found = DoCLsProcedure(nbkg, sigma_nbkg, srbin, ntoys, lsig, obs, bkgOnly_events, sPlusb_events);
		cout << endl;
		if(ul_found){
			ofile->cd();
			sPlusb_events.Write();
			break;
		}
	}
	TLine* obsline = new TLine(obs, 0, obs, 1.);
	ofile->cd();
	ofile->WriteObject(obsline,"obsline");
	string newbkgname = bkgOnly_events.GetName();
	newbkgname = newbkgname.substr(0,newbkgname.find("_lambda"));
	bkgOnly_events.SetName(newbkgname.c_str());
	bkgOnly_events.SetTitle(newbkgname.c_str());
	bkgOnly_events.Write();
	cout << "wrote hists to " << ofilename << endl;
}
