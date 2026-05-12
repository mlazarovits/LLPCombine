import ROOT
import uproot
import mplhep as hep
import matplotlib.pyplot as plt
import argparse
import re
from tools import REGION_LABELS

hep.style.use("CMS")

argparser = argparse.ArgumentParser()
argparser.add_argument("--input","-i",help='ROOT file with CLs+b and CLb histograms',required=True)
args = argparser.parse_args()

infile = args.input
binname = infile[:infile.find("_CLs")]
file = uproot.open(infile)

for key in file.keys():
    if "sPlusb_events" in key:
        sig_hist_name = key
        break
sig_hist = file[sig_hist_name]
bkg_hist = file["bkgOnly_events"]
sUL = sig_hist_name[sig_hist_name.find("_lambda")+7:sig_hist_name.find(";")]
sUL = float(sUL.replace("_","."))

#open file in root to get tline
obs_line = ROOT.TFile.Open(infile).Get("obsline")

fig, ax = plt.subplots()
plt.yscale('log')

obs = obs_line.GetX1()

counts, bins = bkg_hist.to_numpy()
bin_centers = bins[:-1]
mask = (bin_centers >= 0) & (bin_centers <= obs)
ax.fill_between(bin_centers[mask], 0, counts[mask], edgecolor='skyblue', facecolor='none',alpha=0.5,step='post',hatch='///',label="$CL_{b}$")

# 3. Fill only a specific part (e.g., between x = -1 and x = 1)
# Create a mask for the bins within your target range
counts, bins = sig_hist.to_numpy()
bin_centers = bins[:-1]
mask = (bin_centers >= 0) & (bin_centers <= obs)
# Use fill_between to color the selected portion
ax.fill_between(bin_centers[mask], 0, counts[mask], color='yellow', alpha=0.5,step='post',label="$CL_{s+b}$")

hep.histplot(bkg_hist,ax=ax,color='blue',histtype='step',label='b-only')
hep.histplot(sig_hist,ax=ax,color='red',histtype='step',label="s+b")

ymin = 5e-4
ymax = 0.25
ax.set_ylim([ymin, ymax])
plt.plot([obs, obs], [ymin, ymax],label='$N_{obs}$ = '+str(round(obs)),color='green')
plt.legend()
hep.cms.label('Preliminary', data=True, lumi=200, com=13.6)

plt.xlabel("Events")
plt.ylabel("Probability")
plt.text(19, 0.01,"$S_{UL} = $"+str(sUL),size=25)
ch_key = re.match(r"Ch\d*", binname)
ch_key = ch_key.group(0) if ch_key else ""

binlabel = REGION_LABELS[ch_key]
plt.text(19, 0.018,binlabel,size=25)

pltname = f'{binname}_CLs.pdf'
fig.savefig(pltname)
print("Saved figure to",pltname)
