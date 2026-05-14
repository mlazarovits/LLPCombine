from pathlib import Path
import argparse
import subprocess
import ROOT
import json
from tools import ALL_BINS


split_bins = {"Pho" : [], "SV" : [], "Mixed" : []}
def ParseObjectBins(dirname):
    files = Path(dirname).glob("*.txt")
    cardname = [file.name for file in files][0]
    with open(dirname+"/"+cardname,"r") as f:
        for line in f:
            if "shapes * " not in line:
                continue
            binname = line.split(" ")[2]
            if "Pho" in binname and "SV" not in binname:
                split_bins["Pho"].append(binname)
            elif "Pho" in binname and "SV" in binname:
                split_bins["Mixed"].append(binname)
            else:
                split_bins["SV"].append(binname)





argparser = argparse.ArgumentParser()
argparser.add_argument("-d","--directory",help="datacard directory",required=True)
argparser.add_argument("-s","--signal",help="which signal to run over",choices=["gogoGZ","sqsqG"],default="gogoGZ")
argparser.add_argument("--mS",help="run over only this parent sparticle mass",default=None)
argparser.add_argument("--mN2",help="run over only this N2 sparticle mass",default=None)
argparser.add_argument("--mN1",help="run over only this N1 sparticle mass",default=None)
argparser.add_argument("--ctau",help="run over only this ctau",default=None)
argparser.add_argument("-e","--extra",help="extra output file label",default=None)
argparser.add_argument("--mask",help='mask channels',choices=['Pho','SV','Mixed'],default=None)
args = argparser.parse_args()

cmd = "combineTool.py -M AsymptoticLimits --there --parallel 4"
dirs = Path(args.directory).iterdir()

#run asymptotic limits
for idx, d in enumerate(dirs):
    dname = d.name+"/"
    if args.signal not in dname:
        continue
    if args.mS is not None and args.mS not in dname:
        continue
    if args.mN2 is not None and args.mN2 not in dname:
        continue
    if args.mN1 is not None and args.mN1 not in dname:
        continue
    if args.ctau is not None and args.ctau+"/" not in dname:
        continue
    mass = d.name[d.name.find("_")+1:]
    if args.mask is None:
        oname = f"{args.directory}{d.name}/higgsCombine_{mass}.AsymptoticLimits.mH120.root"
    else:
        oname = f"{args.directory}{d.name}/higgsCombine_{mass}_mask{args.mask}.AsymptoticLimits.mH120.root"
    if Path(oname).exists():
        print(f"Skipping {args.directory}{d.name}. AsymptoticLimits file already exists.")
        continue
    print("Running limits for",d.name)
    if args.mask is not None:
        print("Masking",args.mask,"channels")
        #puts the workspace in the same directory as the datacard
        ws_cmd = f"combineTool.py -M T2W -i {args.directory}{d.name}/*.txt -o ws_with_masks.root --channel-masks"
        if not Path(f"{args.directory}{d.name}/ws_with_masks.root").exists():
            print("Making workspace with mask parameters")
            subprocess.run(ws_cmd.split(" "))
        else:
            print(f"Skipping workspace for {args.directory}{d.name}. Already exists.")
        #get binnames from datacard
        ParseObjectBins(f"{args.directory}{d.name}")
        mask_chs = split_bins[args.mask]
        mask_params = ""
        for ch in mask_chs:
            mask_params += f"mask_{ch}=1,"
        mask_params = mask_params[:-1]
        d_cmd = f"{cmd} -n _{mass}_mask{args.mask} -d {args.directory}{d.name}/ws_with_masks.root --setParameters {mask_params}"
    else:
        d_cmd = f"{cmd} -n _{mass} -d {args.directory}{d.name}/*.txt"
    subprocess.run(d_cmd.split(" "))

#reset iterator
dirs = Path(args.directory).iterdir()
#collect limits in json
js_out = {}
dirname = args.directory[args.directory.find("datacards_")+10:].strip("/")
outname = dirname+"_"+args.signal+"_limits"
if args.mS is not None:
    outname += f"_mGl-{args.mS}"
if args.mN2 is not None:
    outname += f"_mN2-{args.mN2}"
if args.mN1 is not None:
    outname += f"_mN1-{args.mN1}"
if args.ctau is not None:
    outname += f"_ctau-{args.ctau}"
if args.mask is not None:
    outname += f"_mask{args.mask}"
for idx, d in enumerate(dirs):
    dname = d.name+"/"
    if args.signal not in dname:
        continue
    if args.mS is not None and args.mS not in dname:
        continue
    if args.mN2 is not None and args.mN2 not in dname:
        continue
    if args.mN1 is not None and args.mN1 not in dname:
        continue
    if args.ctau is not None and args.ctau+"/" not in dname:
        continue
    #assume only 1 asymptotic limit file in each directory
    print("Collecting limits for",d.name)
    limits_name = f"*AsymptoticLimits*.root"
    if args.mask is not None:
        limits_name = f"*mask{args.mask}*AsymptoticLimits*.root"
    asymlimits = [item for item in Path(args.directory+"/"+d.name).glob(limits_name)]
    n_asymlimits = len(asymlimits)
    if(n_asymlimits == 0):
        continue
    if(n_asymlimits > 1):
        print(f"Warning: there are {n_asymlimits} limits root files found in this directory. Will only use the first one found.")
    #get json key from datacard directory
    json_key = d.name[d.name.find("_")+1:]
    file_name = args.directory+"/"+d.name+"/"+asymlimits[0].name
    limit_file = ROOT.TFile(file_name)
    tree = limit_file.Get("limit")
    #from HiggsAnalysis code - toys and limit_err options not implemented
    for evt in tree:
        mh = json_key#str(evt.mh)
        if mh not in js_out:
            js_out[json_key] = {}
        if evt.quantileExpected == -1:
            js_out[mh]["obs"] = evt.limit
        elif abs(evt.quantileExpected - 0.5) < 1e-4:
            js_out[mh]["exp0"] = evt.limit
        elif abs(evt.quantileExpected - 0.025) < 1e-4:
            js_out[mh]["exp-2"] = evt.limit
        elif abs(evt.quantileExpected - 0.160) < 1e-4:
            js_out[mh]["exp-1"] = evt.limit
        elif abs(evt.quantileExpected - 0.840) < 1e-4:
            js_out[mh]["exp+1"] = evt.limit
        elif abs(evt.quantileExpected - 0.975) < 1e-4:
            js_out[mh]["exp+2"] = evt.limit
# print js_out
jsondata = json.dumps(js_out, sort_keys=True, indent=2, separators=(",", ": "))
# print jsondata
if args.extra is not None:
    outname += f"_{args.extra}"
outname += ".json"
with open(outname, "w") as out_file:
    out_file.write(jsondata)
print("Wrote limits to",outname)
