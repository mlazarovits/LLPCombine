import re
import yaml
from pathlib import Path
import subprocess
import json
import shutil
import argparse

def sort_nested_dict(d: dict) -> dict:
    """Recursively sorts nested dictionary keys alphabetically."""
    return {
        key: sort_nested_dict(value) if isinstance(value, dict) else value 
        for key, value in sorted(d.items())
    }
class QuotedString(str):
    pass

def quoted_string_representer(dumper, data):
    return dumper.represent_scalar(
        "tag:yaml.org,2002:str",
        data,
        style='"'
    )

yaml.SafeDumper.add_representer(
    QuotedString,
    quoted_string_representer
)

def quote_all_strings(obj):
    if isinstance(obj, dict):
        for key, value in obj.items():
            if isinstance(value, str) and not isinstance(value, QuotedString):
                obj[key] = QuotedString(value)
            else:
                quote_all_strings(value)

    elif isinstance(obj, list):
        for i, value in enumerate(obj):
            if isinstance(value, str) and not isinstance(value, QuotedString):
                obj[i] = QuotedString(value)
            else:
                quote_all_strings(value)


class FlowList(list):
    pass


class CustomDumper(yaml.SafeDumper):
    def increase_indent(self, flow=False, indentless=False):
        return super().increase_indent(
            flow, indentless=False
        )

def flow_list_representer(dumper, data):
    return dumper.represent_sequence(
        "tag:yaml.org,2002:seq",
        data,
        flow_style=True
    )


CustomDumper.add_representer(
    FlowList, flow_list_representer
)

CustomDumper.add_representer(
    QuotedString, quoted_string_representer
)


argparser = argparse.ArgumentParser()
argparser.add_argument("-c","--config",help="reference yaml")
argparser.add_argument("--dryRun",action="store_true",default=False)
argparser.add_argument("--remakeYamls",action="store_true",default=False)
argparser.add_argument("--remakeBFIs",action="store_true",default=False)
args = argparser.parse_args()
dryrun = args.dryRun
remakeyamls = args.remakeYamls
remakebfis = args.remakeBFIs
ref_yaml = args.config


with open(ref_yaml,"r") as f:
    data = yaml.safe_load(f)

#maybe move to input arguments since different mass points (and full/fastsim) have different simulated lifetimes
#defined for photon BR, Z = 1 - BRphoton
sampleToTargetBRs = {
                        0.5:[0,0.25,0.5,0.75,1],
                    }

#dont run on config that is already being reweighted
if "decayWeights" in data.keys():
    print("lifetime reweighting already done in yaml",ref_yaml)
    exit()

yamlname = ref_yaml[ref_yaml.rfind("/")+1:ref_yaml.find(".yaml")]
ctau_yaml_dir = Path(f"{Path(ref_yaml).parent}/{yamlname}_BRReweightedConfigs/")
ctau_yaml_dir.mkdir(parents=True,exist_ok=True)
 

for sampleBR, targetBRs in sampleToTargetBRs.items():
    for targetBR in targetBRs:
        new_yaml_file = f"{ctau_yaml_dir}/{yamlname}_{int(targetBR*100)}PhoBR{int((1-targetBR)*100)}ZBR.yaml"
        if Path(new_yaml_file).is_file() and not remakeyamls:
            print(f"AnalysisConfig {new_yaml_file} exists. Skipping.")
            continue
        else:
            print(f"Making AnalysisConfig {new_yaml_file}")
        data["decayWeights"] = {
                                "sampleZrate":1-sampleBR,
                                "sampleGrate":sampleBR,
                                "targetZrate":1-targetBR,
                                "targetGrate":targetBR
                                }
        #only recreate BFIs for signal (leave data in original config)
        data["samples"].pop("data",None)
        outputjson = data["analysis"]["output_json"]
        splitidx = outputjson.find(".json")
        for match in re.finditer(r"_Z\d+G\d+", outputjson):
            splitidx = match.start()
            break
        outputjson = outputjson[:splitidx]
        outputjson += f"_Z{int((1-targetBR)*100)}G{int(targetBR*100)}.json"
        data["analysis"]["output_json"] = outputjson
        data["samples"]["signals"] = FlowList(
           data["samples"]["signals"]
        )
        # Move decayWeights to immediately after samples
        if "decayWeights" in data:
            data = {
                key: value
                for key, value in data.items()
                if key != "decayWeights"
            }
        
            reordered_data = {}
            for key, value in data.items():
                reordered_data[key] = value
        
                if key == "samples":
                    reordered_data["decayWeights"] = {
                                "sampleZrate":1-sampleBR,
                                "sampleGrate":sampleBR,
                                "targetZrate":1-targetBR,
                                "targetGrate":targetBR
                                }
        
            data = reordered_data
        # Wrap every string value
        quote_all_strings(data)
        with open(new_yaml_file,"w") as f:
            yaml.dump(
                data,
                f,
                Dumper=CustomDumper,
                sort_keys=False,
                default_flow_style=False,
                width=100000
            )
            #yaml.safe_dump(data,f,sort_keys=False,default_flow_style=False,width=100000)
        #print("Wrote new yaml",new_yaml_file)
#run BFIs over all relevant yamls (ref and in Ctau dir)
bfis = []
ref_bfi = ""
#copy over ref yaml into ctau_yaml_dir to include it in BFI making
shutil.copy(ref_yaml,ctau_yaml_dir)
for filepath in ctau_yaml_dir.iterdir():
    if filepath.is_file():
        bfi = ""
        with open(filepath,"r") as f:
            data = yaml.safe_load(f)
            bfi = "json/"+data["analysis"]["output_json"]
            #brlabel = "Z50G50" #default
            #if "decayWeights" in data.keys():
            #    zlabel = str(int(data["decayWeights"]["targetZrate"])*100)
            #    gamlabel = str(int(data["decayWeights"]["targetGrate"])*100)
            #    brlabel = f"Z{zlabel}G{gamlabel}"
            #bfi += f"_{brlabel}.json"
        if filepath.name == ref_yaml.split("/")[1]:
            if "decayWeights" not in data.keys(): #default BRs
                bfi = bfi.removesuffix(".json")+"_Z50G50.json"
            ref_bfi = bfi
        if bfi not in bfis:
            bfis.append(bfi)
        if Path(bfi).is_file() and not remakebfis:
            print(f"BFI json {bfi} already exists. Skipping.")
            continue
        else:
            print(f"Making BFI {bfi}")
        cmd = f"./BFI.x -c {filepath}"
        if dryrun:
            print(cmd)
        else:
            result = subprocess.run(cmd.split(" "),capture_output=True,text=True)
# Concatenate BFI JSONs into one
tot_bfi = {}
#print("ref_bfi",ref_bfi)
for bfi in bfis:
    if not Path(bfi).is_file():
        print(f"{bfi} does not exist. Skipping.")
        continue
    if args.dryRun:
        print(f"Adding {bfi} to total BFI")
    with open(bfi, "r") as f:
        data = json.load(f)

    for region, contents in data.items():
        if region not in tot_bfi:
            tot_bfi[region] = {}

        # Save all entries except "data"
        tot_bfi[region].update({
            key: value
            for key, value in contents.items()
            if key != "data"
        })

        # Only save "data" if it contains meaningful, nonzero values
        if "data" in contents and bfi == ref_bfi:
            tot_bfi[region]["data"] = contents["data"]

tot_bfi = sort_nested_dict(tot_bfi)


tot_bfi_name = bfis[0]
for match in re.finditer(r"_Z\d+G\d+", tot_bfi_name):
    splitidx = match.start()
    break
tot_bfi_name = tot_bfi_name[:splitidx]
tot_bfi_name += "_BRScan.json"
with open(tot_bfi_name,"w") as outfile:
    json.dump(tot_bfi, outfile, indent=4)
print(f"Wrote overall ctau scan BFI to {tot_bfi_name}")
