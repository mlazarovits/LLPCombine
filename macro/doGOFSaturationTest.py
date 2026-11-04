import argparse
import subprocess
import ROOT
import mplhep as hep
import matplotlib.pyplot as plt
import numpy as np

hep.style.use("CMS")

argparser = argparse.ArgumentParser()
argparser.add_argument("--obs",help="root file with observed test statistic",default=None)
argparser.add_argument("--toys",help="root file with test statistics from toys",default=None)
argparser.add_argument("--datacard",help="datacard with model for saturation test",default=None)
argparser.add_argument("--nToys",help="number of toys to throw for GOF test statistic",default=1000)
argparser.add_argument("--seed",help="seed for toys to throw for GOF test statistic",default=12456) #default seed in higgscombine

args = argparser.parse_args()

if args.datacard is None and ((args.obs is None) or (args.toys is None)):
    print("Pass datacard for GOF test with --datacard [path/to/datacard]")
    exit()
#run observed test statistic
match = "datacards"
oname = None
if args.obs is None:
    oname = args.datacard[args.datacard.find(match)+len(match):args.datacard.find("/")]
    obs_oname = f"{oname}_Obs"
    cmd = f"combine -M GoodnessOfFit {args.datacard} --algo=saturated -n {obs_oname}"
    print("Running observed test statistic")
    subprocess.run(cmd.split(" "))
    obs_file = f"higgsCombine{obs_oname}.GoodnessOfFit.mH120.root"
else:
    obs_file = args.obs


if args.toys is None:
    oname = args.datacard[args.datacard.find(match)+len(match):args.datacard.find("/")]
    toys_oname = f"{oname}_Obs"
    cmd = f"combine -M GoodnessOfFit {args.datacard} --algo=saturated -n {toys_oname} --toys {args.nToys} --seed {args.seed}"
    print("Running toys for test statistic distribution")
    subprocess.run(cmd.split(" "))
    toys_file = f"higgsCombine{toys_oname}.GoodnessOfFit.mH120.{seed}.root"
else:
    toys_file = args.toys

#get obs test stat
obs_f = ROOT.TFile.Open(obs_file)
obs_tree = obs_f.Get("limit")
#get first entry
next(iter(obs_tree))
obs_stat = obs_tree.limit

#load toys
rdf_toys = ROOT.RDataFrame("limit",toys_file)
toys_vals = rdf_toys.AsNumpy(columns=["limit"])
#play around with hist bounds for appropriate hist binning
hist_min = max(0,obs_stat * 0.6) 
hist_max = min(obs_stat * 4, 1e6)
nbins = 50
test_stat_hist = rdf_toys.Histo1D(("test_stat","test_stat",nbins,hist_min,hist_max),"limit")
#execute event loop
rdf_toys.Report()

#normalize hist
test_stat_hist.Scale(1./test_stat_hist.Integral())
#get p-value from obs test stat
stat_bin = test_stat_hist.FindBin(obs_stat)
p_val = test_stat_hist.Integral(stat_bin,nbins+2) #right tail

fig, ax = plt.subplots()
ymin = 5e-4
ymax = 1.
ax.set_ylim([ymin, ymax])
plt.yscale('log')
#draw everything
#shading
counts = np.frombuffer(test_stat_hist.GetArray(), dtype=np.float64, count=nbins+2)[1:-1]
edges = np.array([test_stat_hist.GetXaxis().GetBinLowEdge(i) for i in range(1, nbins + 2)])
bin_centers = edges[:-1]
mask = (bin_centers >= obs_stat)
ax.fill_between(bin_centers[mask], 0, counts[mask], edgecolor='skyblue', facecolor='none',alpha=0.5,step='post',hatch='///',label="")


hep_stat_hist = hep.histplot(test_stat_hist,ax=ax,histtype='step',label='toys')
plt.plot([obs_stat, obs_stat], [ymin, ymax],label='$t_{obs}$ = '+str(round(obs_stat,2)),color='black')
plt.legend()
hep.cms.label('Preliminary', data=True, lumi='424', com=13.6)
plt.xlabel("Test Statistic $t = -2ln(\mathcal{L}_\mathrm{sat} / \mathcal{L})$")
plt.ylabel("a.u.")

#print pval on plot
xtext = obs_stat * 1.1
ytext = 0.4
plt.text(xtext, ytext-0.2,f"p-val: {round(p_val,2)}",size=25)

if oname is None:
    match = "higgsCombine_"
    pltname = args.obs[args.obs.find(match)+len(match):args.obs.find(".GoodnessOfFit")]
    pltname = f"gofSaturatedTest_{pltname}.pdf"
else:
    pltname = f"gofSaturatedTest_{oname}.pdf"
        
print("Saved GOF plot to",pltname)
fig.savefig(pltname)


