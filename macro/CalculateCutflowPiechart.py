from FileProcessor import FileProcessor
import ROOT
import yaml
from pathlib import Path
import awkward as ak
from tools import EfficiencyParser
import argparse
import re

COLORS = {
        "CRGeLep1": ROOT.kBlue, 
        "CRGeHad1": ROOT.kCyan+4,
        "CRgeq1PhoBHEarlyDxyHadLow": ROOT.kOrange+7, 
        "CRgeq1PhoBHLateDxyHadLow":ROOT.kOrange,
        "CRgeq1PhonotBHEarlyDxyHadLow":ROOT.kOrange+10,
        "CRgeq1PhoMedIsoPromptSVLowDxy": ROOT.kYellow+1,
        "CRgeq1PhoBHEarly": ROOT.kSpring-5, 
        "CRgeq1PhoBHLate": ROOT.kGreen,
        "CRgeq1PhoNotBHEarly": ROOT.kCyan, 
        "CReq1PhoMedIsoPrompt":  ROOT.kTeal-1, 
        "CReq2PhoMedIsoPrompt": ROOT.kTeal-7, 
        "SRgeq1PhonotBHLateDxyHadHigh": ROOT.kViolet-3,
        "SRgeq1PhoTightIsoPromptSVHighDxy": ROOT.kViolet+1, 
        "SReq2PhoTightIsoPrompt": ROOT.kMagenta, 
        "SReq1PhoTightIsoPrompt": ROOT.kPink+1, 
        "SRgeq1PhoNotBHLate": ROOT.kMagenta-9,
        "SRGeLep1": ROOT.kBlue-1,
        "SRGeHad1": ROOT.kAzure+1,
}


def flatten_cutstring(cutlist):
    flatlist = ak.flatten(cutlist,axis = None) 
    return ' && '.join(flatlist)

def GetTotalMassSplit(filename):
    mgl = filename[filename.find("mGl-")+4:]
    mgl = float(mgl[:mgl.find("_")])
    mn1 = filename[filename.find("mN1-")+4:]
    mn1 = float(mn1[:mn1.find("_")])
    return mgl - mn1


fileprocessor = FileProcessor()
effparser = EfficiencyParser()
sigs = fileprocessor.GetFiles("SMS_gogoGZ")

def main(args):
    yaml_path = '../config_master/BigGuy_NonCompressed_FullRegions_AnalysisConfig.yaml'
    yaml_regions = {}
    
    #assuming baseline and cleaning are the same across yamls
    baseline = None
    cleaning = None
    print("Parsing yaml",yaml_path)
    with open(yaml_path,'r') as f:
        data = yaml.safe_load(f)
    baseline = data['baseline_cuts'][0]
    cleaning = data['Cleaning'][0]
    if 'regions' not in data.keys():
        print("Regions not defined in yaml",yaml_path,"Please define in yaml and rerun")
        exit()
    yaml_regions = data['regions']
   
    if args.SRsonly:
        yaml_regions = {k:  v for k, v in yaml_regions.items() if "SR" in k}

    #create output root file
    ofilename = "cutflowPieCharts"
    if(args.SRsonly):
        ofilename += "_SRsOnly"
    ofilename += ".root" 
    ofile = ROOT.TFile.Open(ofilename,"RECREATE")
    gen_states = ['Evt_isGG','Evt_isGZ','Evt_isZZ']
    
    #for each signal point
    for sig in sigs:
        dmass = GetTotalMassSplit(sig)
        #skip compressed signals (for now)
        if(dmass <= 200):
            continue
        signame = sig[sig.find("mGl-"):sig.find("_rjrskim")]
        signame = signame.replace("-","_")
        print("sig",signame)
        #create rdf
        rdf = ROOT.RDataFrame("kuSkimTree",sig)
        #do loop over gen-level final states
        for final_state in gen_states:
            print("final state",final_state)
            #apply preselection
            presel_name = f'presel_and_{final_state}'
            rdf0 = rdf.Filter(f'{baseline} && {cleaning} && {final_state}',presel_name)
            filtered_rdfs = {}
            rdfs = []
            wt_all = rdf0.Sum("evtFillWgt")
            for name, regdef in yaml_regions.items():
                cutstring = flatten_cutstring(regdef)
                rdf_reg = rdf0.Filter(cutstring, name)
                rdfs.append(rdf_reg)
                
            #execute overall rdf event loop with report
            report = rdf.Report()
            #parse report, extract efficiencies relative to preselection
            lines = effparser.report2str(report)
            denom_info = effparser.get_denom_line(lines, presel_name)
            total_eff = 0
            start_angle = 0
            end_angle = 0
            parsed_effs = []
            for line in lines:
                parsed_eff = effparser.parse_eff_line(line, denom_info)
                parsed_effs.append(parsed_eff)
            parsed_effs.sort(key=lambda x:x[2])
            
            ellipse_slices = []
            leg = ROOT.TLegend(0.05,0.6,0.95,0.95)
            leg.SetNColumns(2)
            color_idx = 0
            for parsed_eff in parsed_effs:
                reg_eff = parsed_eff[2]
                reg_name = re.sub(r'^\d+', '', parsed_eff[0][2:])
                print(parsed_eff)
                if 'presel' in parsed_eff[0]:
                    continue  
                total_eff += reg_eff
                angle_slice = 360 * (reg_eff/100)
                phimin = start_angle
                phimax = start_angle + angle_slice
                r = 0.7
                ell_color = COLORS[reg_name]
                ell = ROOT.TEllipse(0.5, 0, r, r, phimin, phimax)
                if(reg_eff >= 0):
                    ell.SetFillStyle(3003)
                    ell.SetFillColor(ell_color)
                ell.SetLineColor(ell_color)
                leg_name = f"{reg_name}: {reg_eff:.2f}%"
                #print(leg_name)
                leg.AddEntry(ell,leg_name,"l")
                ellipse_slices.append(ell)
                start_angle += angle_slice
                color_idx += 1
            print(f"total efficiency: {total_eff:.2f}%")
            print()
            #create tcanvas
            can = ROOT.TCanvas(f"can_{signame}_{final_state}",f"can_{signame}_{final_state}")
            can.Range(-1,-1,2,2)
            #draw tcanvas
            can.Draw()
            for ellslice in ellipse_slices:
              ellslice.Draw("same")
            leg.Draw("same")
            #save tcanvas to root file
            ofile.cd()
            can.Write()
    print("wrote piecharts to",ofilename)


if __name__ == "__main__":
    argparser = argparse.ArgumentParser()
    argparser.add_argument("--SRsonly",help='only plot SRs in piechart',action='store_true',default=False)
    argparser.add_argument("--lumi",help='luminosity',default=200)
    args = argparser.parse_args()
    main(args)
