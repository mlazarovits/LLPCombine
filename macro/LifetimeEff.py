import ROOT
import yaml
from FileProcessor import FileProcessor
#from CalculateCutflowPiechart import COLORS
import json
import matplotlib.pyplot as plt
import numpy as np
import re
import mplhep as hep
import argparse
import glob 

hep.style.use("CMS")



COLORS = {
        "Ch1" : "purple",
        "Ch2" : "purple",
        "SVonly_SR" : "purple",
        "Ch3" : "darkviolet",
        "Ch4" : "darkviolet",
        "Ch5" : "olive",
        "Ch6" : "limegreen",
        "Ch7" : "forestgreen",
        "Ch8" : "darkgreen",
        "DelPho_NotBHLateSR" : "darkgreen",
        "Ch9" : "greenyellow",
        "Ch10" : "greenyellow",
        "Ch11" : "mediumseagreen",
        "Ch12" : "mediumseagreen",
        "Eq2Pho_TightIsoSR" : "mediumseagreen",
        "Ch16": "palevioletred",
        "MixDel_NotBHLateSR": "palevioletred",
        "MixPrompt_SR": "deeppink",
}


argparser = argparse.ArgumentParser()
argparser.add_argument("--mGl",help="run over only this parent sparticle mass",default=None)
argparser.add_argument("--mN2",help="run over only this N2 sparticle mass",default=None)
argparser.add_argument("--mN1",help="run over only this N1 sparticle mass",default=None)
argparser.add_argument("-e","--extra",help="extra output file label",default=None)
args = argparser.parse_args()
args.SRsonly = True
#TODO - implement efficiency wrt presel (need presel numbers in BFI json)

signal_mass = f'gogoGZ_{args.mGl}_{args.mN2}_{args.mN1}'
lumi = 200
compressed = False
if float(args.mGl) - float(args.mN2) <= 100 and float(args.mN2) - float(args.mN1) <= 100:
    jsonname = "../json/LittleGuy_Compressed_FullRegions"
    compressed = True
else:
    jsonname = "../json/BigGuy_NonCompressed_FullRegions"

ctau_files = glob.glob(f"{jsonname}_Ctau*_Z50G50.json")
ctau_files.append(f"{jsonname}_Z50G50.json")

ctau_ch_evts = {}
ch_names = {}
for BFIfile in ctau_files:
    print(BFIfile)
    data = {}
    with open(BFIfile,"r") as f:
        data = json.load(f)
    
    for reg_name, region in data.items():
        if args.SRsonly and "SR" not in reg_name:
            continue
        for signal, evts in region.items():
            if signal_mass not in signal:
                continue
            lifetime = signal.split("_")[-1]
            if lifetime not in ctau_ch_evts.keys():
                ctau_ch_evts[lifetime] = {}
                ctau_ch_evts[lifetime]['denom'] = [0.,0.]
            #print("signal",signal)
            wt_evt = region[signal][1]
            wt_err = region[signal][2]
            ch_key = re.match(r"Ch\d*", reg_name)
            ch_key = ch_key.group(0) if ch_key else ""
            if ch_key == "":
                ch_key = reg_name
            #ch_key += f"_{lifetime}"
            #overwrite 'anchor' CR regions
            if ch_key != reg_name:
                ch_name = reg_name[reg_name.find(ch_key)+len(ch_key):-2]
            else:
                ch_name = reg_name[:-2]
                ch_key = ch_name
            if "Bin" in ch_name:
                ch_name = ch_name[:ch_name.find("Bin")]
            if args.SRsonly and not compressed:
                ch_name = ch_name[ch_name.find("SR")+2:]
            ch_names[ch_key] = ch_name
            if ch_key not in ctau_ch_evts[lifetime].keys():
                ctau_ch_evts[lifetime][ch_key] = {}
                ctau_ch_evts[lifetime][ch_key] = [wt_evt, wt_err]
            else:
                cur_evts = ctau_ch_evts[lifetime][ch_key]
                new_err = np.sqrt(cur_evts[1]**2 + wt_err**2) #add errors in quadrature, add weighted events
                ctau_ch_evts[lifetime][ch_key] = [cur_evts[0] + wt_evt, new_err]
            if args.SRsonly: 
                cur_evts = ctau_ch_evts[lifetime]['denom']
                new_err = np.sqrt(cur_evts[1]**2 + wt_err**2) #add errors in quadrature, add weighted events
                #print('ctau',lifetime,ch_key,"denom",ctau_ch_evts[lifetime]['denom'][0],'evts',wt_evt)
                ctau_ch_evts[lifetime]['denom'] = [cur_evts[0] + wt_evt, new_err]

#effs[ch] = [[ctau, eff]]
effs = {}
for lifetime, ch_evts in ctau_ch_evts.items():
    for ch, evts in ch_evts.items():
        if ch == "denom":
            continue
        if ch not in effs.keys():
            effs[ch] = []
        #print("ch",ch,"lifetime",lifetime,"evts",evts[0],"denom",ch_evts[f"denom"][0])
        effs[ch].append(np.array([float(lifetime), float(evts[0] / ch_evts[f"denom"][0])]))

fig, ax = plt.subplots(figsize=(10,8))
hep.cms.label("Preliminary", data = False, lumi=None,com=13.6)
for ch, val in effs.items():
    effs[ch] = np.array(sorted(val,key=lambda x : x[0]))
    ax.plot(effs[ch][:,0], effs[ch][:,1],label=ch_names[ch],linestyle='solid',color=COLORS[ch],marker='o')

ax.set_xlabel(r'$c\tau$ (cm)')
ax.set_ylabel("efficiency")
plt.xscale('log')
plt.ylim(-0.1,1.1)
#plt.legend(fontsize=12)
plot_signal_label = "$m_{\\tilde{g}}=$"+args.mGl+" GeV, $m_{\chi^0_2}=$" + args.mN2 + " GeV, $m_{\chi^0_1}=$"+args.mN1+" GeV"

legloc = 'best'
if compressed:
    legloc = 2
leg = ax.legend(ncol=2,fontsize=15,loc=legloc)
#force draw to get legend location
fig.canvas.draw()

#get axis info
inverse_transform = ax.transAxes.inverted()
bbox = leg.get_frame().get_window_extent()
bottom_left_axes = inverse_transform.transform((bbox.x0, bbox.y0))
text_x = bottom_left_axes[0]
text_y = bottom_left_axes[1] - 0.05  # Subtract a small padding offset

plt.text(text_x,text_y, plot_signal_label, fontsize=15,transform=ax.transAxes)
if args.SRsonly:
    plt.text(text_x,text_y - 0.05,"SR Breakdown",fontsize=15, transform=ax.transAxes)

pltname = f"lifetimeEff_gogoGZ_mGl-{args.mGl}_mN2-{args.mN2}_mN1-{args.mN1}.pdf"
plt.savefig(pltname)
print('saved to',pltname)

#correct error with binomial error, sigma_e = sqrt(e(1-e)/n_eff) for n_eff = (sum w_i)**2 / sum (w_i**2) over all events in denom i
#n_eff = (sum w_i)**2 / sum (w_i**2) = n_wt^2 / sigma_stat^2 where sigma_stat = sqrt(sum_i sigma_stat,i^2) over bins i




