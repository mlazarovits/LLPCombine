import ROOT
import yaml
from FileProcessor import FileProcessor
import json
import matplotlib.pyplot as plt
import numpy as np
import re
import mplhep as hep
import argparse

hep.style.use("CMS")

COLORS = {
        "Ch1" : "lightcoral",
        "Ch2" : "lightcoral",
        "Ch3" : "steelblue",
        "Ch4" : "steelblue",
        "Ch5" : "olive",
        "Ch6" : "limegreen",
        "Ch7" : "forestgreen",
        "Ch8" : "seagreen",
        "Ch9" : "navy",
        "Ch10" : "navy",
        "Ch11" : "darkorange",
        "Ch12" : "darkorange",
        "base" : "fuschia",
        "preselection" : "hotpink",
}


argparser = argparse.ArgumentParser()
argparser.add_argument("--mGl",help="run over only this parent sparticle mass",default=None)
argparser.add_argument("--mN2",help="run over only this N2 sparticle mass",default=None)
argparser.add_argument("--mN1",help="run over only this N1 sparticle mass",default=None)
argparser.add_argument("--ctau",help="run over only this ctau",default=None)
argparser.add_argument("-e","--extra",help="extra output file label",default=None)
args = argparser.parse_args()


BFIfile = "../json/BigGuy_NonCompressed_FullRegions_Cutflow.json"
signal = f'gogoGZ_{args.mGl}_{args.mN2}_{args.mN1}_{args.ctau}'
lumi = 200


data = {}
with open(BFIfile,"r") as f:
    data = json.load(f)

ch_evts = {}
ch_names = {}
for reg_name, region in data.items():
    if signal not in region:
        print("Signal",signal,"not found in json",BFIfile,"Exiting.")
        exit()
    wt_evt = region[signal][1]
    wt_err = region[signal][2]
    ch_key = re.match(r"Ch\d*", reg_name)
    ch_key = ch_key.group(0) if ch_key else ""
    if ch_key == "":
        ch_key = reg_name
    #overwrite 'anchor' CR regions
    ch_names[ch_key] = reg_name[:-2]
    if ch_key not in ch_evts.keys():
        ch_evts[ch_key] = {}
        ch_evts[ch_key] = [wt_evt, wt_err]
    else:
        cur_evts = ch_evts[ch_key]
        new_err = np.sqrt(cur_evts[1]**2 + wt_err**2) #add errors in quadrature, add weighted events
        ch_evts[ch_key] = [cur_evts[0] + wt_evt, new_err]

#binomial error, sigma_e = sqrt(e(1-e)/n_eff) for n_eff = (sum w_i)**2 / sum (w_i**2) over all events in denom i
#n_eff = (sum w_i)**2 / sum (w_i**2) = n_wt^2 / sigma_stat^2 where sigma_stat = sqrt(sum_i sigma_stat,i^2) over bins i

denom_name = "preselection"
denom = ch_evts[denom_name] #18089 * 1.667e-05 * 200 #total in skim for this signal * lumi 
base_denom = ch_evts["base"]

fig, ax = plt.subplots(figsize=(10,8))
npresel = sum(1 for key in ch_evts if "Ch" not in key)

ch_evts = dict(sorted(ch_evts.items(), key=lambda x : x[1], reverse=True))

xidx = 0
xaxislabels = []
xbin_size = 0.5
#presel only
for ch, evts in ch_evts.items():
    if "Ch" in ch:
        continue
    denom_wt_evts = base_denom[0]
    denom_wt_err = base_denom[1]

    eff = evts[0] / denom_wt_evts
    #number of effective events
    neff = (denom_wt_evts**2) / (denom_wt_err**2)
    #binomial error on efficiency
    bin_err = np.sqrt(eff*(1 - eff)/neff)
    
    label = ch#ch_names[ch]

    color = ch in COLORS.keys() ? COLORS[ch] : 'k' 

    ax.errorbar(xidx,eff,xerr=xbin_size,yerr = bin_err, marker='s', markersize=7, color = 'k')
    xaxislabels.append(ch)
    xidx += 1

#regions only
for ch, evts in ch_evts.items():
    if "Ch" not in ch:
        continue
    denom_wt_evts = denom[0]
    denom_wt_err = denom[1]

    eff = evts[0] / denom_wt_evts
    #number of effective events
    neff = (denom_wt_evts**2) / (denom_wt_err**2)
    #binomial error on efficiency
    bin_err = np.sqrt(eff*(1 - eff)/neff)
    
    label = ch#ch_names[ch]

    if "CR" in ch_names[ch]:
        marker = ['o',7]
    else:
        marker = ['*',10]
    ax.errorbar(npresel+1,eff,xerr = xbin_size,yerr = bin_err, marker=marker[0],label=label, markersize=marker[1], color = COLORS[ch])
    xaxislabels.append("")
#plt.xticks([i for i in ch_evts],xaxislabels)
hep.cms.label("Preliminary", data = False, lumi=lumi,com=13.6)
plt.xticks(np.arange(len(ch_evts)),xaxislabels,size=15)
xmin = -0.6
plt.xlim(xmin,npresel+2)
plt.ylabel("Efficiency")
#plt.title(f"Efficiency Cutflow: {signal}",size=20)
plt.ylim(-0.1,1.25)

plot_signal_label = signal.split("_")
plot_signal_label = "$m_{\\tilde{g}}=$"+plot_signal_label[1]+" GeV, $m_{\chi^0_2}=$" + plot_signal_label[2] + " GeV\n$m_{\chi^0_1}=$"+plot_signal_label[3]+" GeV, $c\\tau=$"+plot_signal_label[4]+" cm"

plt.text(xmin+0.3,1.05, plot_signal_label, fontsize=15)

ax.legend(ncol=2,fontsize=15)
ofilename = f"cutflow_{signal}.pdf"
plt.savefig(ofilename)
print("Wrote cutflow to",ofilename)



