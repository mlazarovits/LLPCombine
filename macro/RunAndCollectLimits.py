from pathlib import Path
import argparse
import subprocess
import ROOT
import json

argparser = argparse.ArgumentParser()
argparser.add_argument("-d","--directory",help="datacard directory",required=True)
argparser.add_argument("-s","--signal",help="which signal to run over",choices=["gogoGZ","sqsqG"])
argparser.add_argument("-e","--extra",help="extra output file label",default=None)
args = argparser.parse_args()

cmd = "combineTool.py -M AsymptoticLimits -t -1 --there --parallel 4"
dirs = Path(args.directory).iterdir()
outname = args.signal+"_limits"

#run asymptotic limits
for idx, d in enumerate(dirs):
    if args.signal not in d.name:
        continue
    print("Running limits for",d.name)
    mass = d.name[d.name.find("_")+1:]
    d_cmd = f"{cmd} -n _{mass} -d {args.directory}{d.name}/*.txt"
    subprocess.run(d_cmd.split(" "))

#reset iterator
dirs = Path(args.directory).iterdir()
#collect limits in json
js_out = {}
for idx, d in enumerate(dirs):
    if args.signal not in d.name:
        continue
    #assume only 1 asymptotic limit file in each directory
    print("Collecting limits for",d.name)
    asymlimits = [item for item in Path(args.directory+"/"+d.name).glob(f"*AsymptoticLimits*.root")]
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
