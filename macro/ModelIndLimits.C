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

bool DoCLsProcedure(double nbkg, double sigma_nbkg, string srbin, int ntoys, double lambda_sig, double& obs, TH1D& bkgOnly_events, TH1D& sPlusb_events){
	TRandom3 rng;
	//create histogram for b-only distribution of events
	string bkghistname = srbin+"_bkgOnly_events_lambda"+dbl2str(lambda_sig,1);
	string sighistname = srbin+"_sPlusb_events_lambda"+dbl2str(lambda_sig,1);
	int nbins = int(nbkg + 7*nbkg);
	if(nbins < 10)
		nbins = 20;
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
		//double lambda_bkg = exp(rng.Gaus(lognorm_mu, lognorm_sig2));
		double lambda_bkg = rng.Gaus(nbkg, sigma_nbkg);
		while(lambda_bkg <= 0){
			lambda_bkg = rng.Gaus(nbkg, sigma_nbkg);
		}
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
	//set obs to be mean of bkg events
	obs = ceil(bkgOnly_events.GetMean());
	//cout << "bkgOnly_events integral " << bkgOnly_events.Integral() << " nentries " << bkgOnly_events.GetEntries() << " sPlusb_events " << sPlusb_events.Integral() << endl;
	//normalize histograms to make PDF (should be 1/ntoys for both)
	if(bkgOnly_events.Integral() > 0) bkgOnly_events.Scale(1/bkgOnly_events.Integral());
	if(sPlusb_events.Integral() > 0) sPlusb_events.Scale(1/sPlusb_events.Integral());

	//get p-values (integrals) from observed values
	int sPlusBbin = sPlusb_events.FindBin(obs);
	double CLsb = sPlusb_events.Integral(0,sPlusBbin);

	int bkgOnlybin = bkgOnly_events.FindBin(obs);
	double CLb = bkgOnly_events.Integral(0,bkgOnlybin);
//cout << "CLsb " << CLsb << " CLb " << CLb << " CLs " << CLsb / CLb << endl;
	bool ul_found = false;
	if(CLsb / CLb <= 0.05){
		//cout << "Upper limit found!! CLs = " << CLsb / CLb << " Nsig events: " << lambda_sig << endl;
		ul_found = true;
	}
	return ul_found;
}


void ModelIndLimits(){
	int ntoys = 100000;

	map<string, string> ch_map;
	ch_map["Ch2"] = "Ch2SRGeLep111";
	ch_map["Ch4"] = "Ch4SRGeHad121";
	ch_map["Ch8"] = "Ch8SRgeq1PhoNotBHLateTightIso11";
	ch_map["Ch10"] = "Ch10SReq1PhoTightIsoPrompt12";
	ch_map["Ch12"] = "Ch12SReq2PhoTightIsoPrompt11";
	ch_map["Ch14"] = "Ch14SRgeq1SVHighDxygeq1PhoNotBHLate01";
	ch_map["DelPho_NotBHLateSR"] = "DelPho_NotBHLateSR10";
	ch_map["Eq2Pho_TightIsoSR"] = "Eq2Pho_TightIsoSR30";
	ch_map["MixDel_SR"] = "MixDel_SR20";
	ch_map["MixPrompt_SR"] = "MixPrompt_SR20";
	ch_map["SVonly_SR"] = "SVonly_SR20";


	map<string,string> reg_labels;
        reg_labels["Ch1"] ="$\mathrm{SV}^{CR, Sxy-}_{mathrm{lep}}$";
        reg_labels["Ch2"] ="$\mathrm{SV}^{SR, Sxy+}_{mathrm{lep}}$";
        reg_labels["Ch3"] ="$\mathrm{SV}^{CR, Sxy-}_{mathrm{had}}$";
        reg_labels["Ch4"] ="$\mathrm{SV}^{SR, Sxy+}_{mathrm{had}}$";
        reg_labels["SVonly_SR"] ="$\mathrm{SV}^{SR, Sxy+}$";
        reg_labels["Ch5"] ="$\gamma^{CR, BH}_{t-}$";
        reg_labels["Ch6"] ="$\gamma^{CR, BH}_{t+}$";
        reg_labels["Ch7"] ="$\gamma^{CR, !BH}_{t-}$";
        reg_labels["Ch8"] ="$\gamma^{SR, !BH}_{t+}$";
        reg_labels["DelPho_NotBHLateSR"] ="$\gamma^{SR, !BH}_{t+}$";
        reg_labels["Ch10"] ="$1 \gamma^{SR, Tight Iso}_{t0}$";
        reg_labels["Ch12"] ="$2 \gamma^{SR, Tight Iso}_{t0}$";
        reg_labels["Eq2Pho_TightIsoSR"] ="$2 \gamma^{SR, Tight Iso}_{t0}$";
        reg_labels["Ch13"] ="$\mathrm{SV}^{CR, Sxy-}_{mathrm{had}}, \gamma_{t+}^{SR, !BH}$";
        reg_labels["Ch14"] ="$\mathrm{SV}^{SR, Sxy+}_{mathrm{had}}, \gamma_{t+}^{SR, !BH}$";
        reg_labels["MixDel_SR"] ="$\mathrm{SV}^{SR, Sxy+}_{mathrm{had}}, \gamma_{t+}^{SR, !BH}$";
        reg_labels["MixPrompt_SR"] ="$\mathrm{SV}^{SR, Sxy+}_{mathrm{had}}, \gamma_{t0}^{SR, Tight Iso}$";
        reg_labels["Ch15"] ="$\gamma^{CR, Med Iso}_{t-}$";
        reg_labels["Ch16"] ="$\gamma^{CR, Med Iso}_{t+}$";
        reg_labels["Ch17"] ="$\gamma^{CR, Tight Iso}_{t-}$";



	string infilename = "FitDiagnostics/fitDiagnostics_LittleGuy_Compressed_FullRegions_v52.root";
	//string infilename = "FitDiagnostics/fitDiagnostics_BigGuy_NonCompressed_FullRegions_TFBeamHalo.root";
	cout << "Using FitDiagnostics: " << infilename << endl;
	TFile* infile = TFile::Open(infilename.c_str(),"READ");
	string ofilename = "ModelIndHists.root";
	TFile* ofile = TFile::Open(ofilename.c_str(),"RECREATE");
	cout << "\\begin{table}" << endl;
	cout << "\\centering" << endl;
	cout << "\\begin{tabular}{c | c | c | c | c}" << endl;
	cout << "Region & $N^{\\text{pred}}_{\\text{bkg}}$ & $\\sigma(N^{\\text{pred}}_{\\text{bkg}})$ & $N_{\\text{obs}}$ & $\\text{S}_{\\text{UL}}$ \\\\" << endl;
	cout << "\\hline \\hline" << endl;
	for(auto chit = ch_map.begin(); chit != ch_map.end(); chit++){
		string srbin = chit->second;
		//get bkg-only post-fit Nbkg and sigma(Nbkg) from fit diagnostics
		TH1D* bonly_postfit_bkg = (TH1D*)infile->Get(("shapes_fit_b/"+srbin+"/total_background").c_str());
		if(bonly_postfit_bkg == nullptr)
			continue;
		double nbkg = bonly_postfit_bkg->GetBinContent(1);
		double sigma_nbkg = bonly_postfit_bkg->GetBinError(1);
		//get observation
		double x, obs;
		TGraphAsymmErrors* data = (TGraphAsymmErrors*)infile->Get(("shapes_fit_b/"+srbin+"/data").c_str());
		data->GetPoint(0, x, obs);

		double d_lsig = 0.1;
		double min_lsig = double(int(nbkg));
		int nsteps = 200;
		double max_lsig = min_lsig + nsteps*d_lsig;

		TH1D bkgOnly_events, sPlusb_events;
		double lsig;
		for(lsig = min_lsig; lsig < max_lsig; lsig += d_lsig){
			bool ul_found = DoCLsProcedure(nbkg, sigma_nbkg, srbin, ntoys, lsig, obs, bkgOnly_events, sPlusb_events);
			if(ul_found){
				ofile->cd();
				sPlusb_events.Write();
				break;
			}
		}
		//cout << "bin " << srbin << " Nbkg " << nbkg << " sigma_nbkg " << sigma_nbkg << " obs " << obs << endl;
		//cout << endl;
		TLine* obsline = new TLine(obs, 0, obs, 1.);
		ofile->cd();
		ofile->WriteObject(obsline,(srbin+"_obsline").c_str());
		string newbkgname = bkgOnly_events.GetName();
		newbkgname = newbkgname.substr(0,newbkgname.find("_lambda"));
		bkgOnly_events.SetName(newbkgname.c_str());
		bkgOnly_events.SetTitle(newbkgname.c_str());
		bkgOnly_events.Write();
		cout << std::setprecision(2) << reg_labels[chit->first] << " & " << nbkg << " & " << sigma_nbkg << " & " << obs << " & " << lsig << " \\\\" << endl;
	}
	cout << "\\end{tabular}" << endl;
	cout << "\\end{table}" << endl;
	cout << "wrote hists to " << ofilename << endl;
}
