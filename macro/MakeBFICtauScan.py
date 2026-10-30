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
argparser.add_argument("--config","-c",help="reference yaml")
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
sampleToTargetCtaus = {
                        10:[1,5],
                        50:[20,30],
                        300:[80,100,200] #only for 2300-1300-100, 1800-1750-1700
                    }

#dont run on config that is already being reweighted
if "lifetimeWeights" in data.keys():
    print("lifetime reweighting already done in yaml",ref_yaml)
    exit()

yamlname = ref_yaml[ref_yaml.rfind("/")+1:ref_yaml.find(".yaml")]
ctau_yaml_dir = Path(f"{Path(ref_yaml).parent}/{yamlname}_CtauReweightedConfigs/")
ctau_yaml_dir.mkdir(parents=True,exist_ok=True)
 

for sampleLifetime, targetLifetimes in sampleToTargetCtaus.items():
    for targetLifetime in targetLifetimes:
        new_yaml_file = f"{ctau_yaml_dir}/{yamlname}_Ctau{sampleLifetime}to{str(targetLifetime).replace('.','p')}.yaml"
        if Path(new_yaml_file).is_file() and not remakeyamls:
            print(f"AnalysisConfig {new_yaml_file} exists. Skipping.")
            continue
        else:
            print(f"Making AnalysisConfig {new_yaml_file}")
        data["lifetimeWeights"] = {"sampleLifetime":sampleLifetime,"targetLifetime":targetLifetime}
        data["samples"]["signals"] = [f"gogoGZ{sampleLifetime}"]
        #only recreate BFIs for signal (leave data in original config)
        data["samples"].pop("data",None)
        outputjson = data["analysis"]["output_json"]
        if 'Ctau' not in outputjson:
            splitidx = outputjson.find(".json")
        else:
            splitidx = outputjson.find("_Ctau")
        outputjson = outputjson[:splitidx]
        outputjson += f"_CtauReweight{sampleLifetime}to{str(targetLifetime).replace('.','p')}.json"
        data["analysis"]["output_json"] = outputjson
        data["samples"]["signals"] = FlowList(
           data["samples"]["signals"]
        )
        # Move lifetimeWeights to immediately after samples
        if "lifetimeWeights" in data:
            data = {
                key: value
                for key, value in data.items()
                if key != "lifetimeWeights"
            }
        
            reordered_data = {}
            for key, value in data.items():
                reordered_data[key] = value
        
                if key == "samples":
                    reordered_data["lifetimeWeights"] = {
                        "sampleLifetime": sampleLifetime,
                        "targetLifetime": targetLifetime,
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
            bfi = "json/"+data["analysis"]["output_json"].removesuffix(".json")
            brlabel = "Z50G50" #default
            if "decayWeights" in data.keys():
                zlabel = str(int(data["decayWeights"]["targetZrate"]*100))
                gamlabel = str(int(data["decayWeights"]["targetGrate"]*100))
                brlabel = f"Z{zlabel}G{gamlabel}"
            if brlabel not in bfi:
                bfi += f"_{brlabel}"
            bfi += ".json"
        if filepath.name == Path(ref_yaml).name:
            #add true reference BFI if no data
            if "data" not in data["samples"].keys() and brlabel != "Z50G50":
                ref_bfi = bfi[:bfi.find("_"+brlabel)]+"_Z50G50.json" 
            else:
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
print("ref_bfi",ref_bfi)
#make sure refbfi exists
if not Path(ref_bfi).is_file():
    ref_bfi_yaml = "config_master/"+filepath.stem[:filepath.stem.find("AnalysisConfig")+14]+".yaml" 
    print(f"Making reference BFI {ref_bfi} from {ref_bfi_yaml}")
    cmd = f"./BFI.x -c {ref_bfi_yaml}"
    if dryrun:
        print(cmd)
    else:
        result = subprocess.run(cmd.split(" "),capture_output=True,text=True)

#make sure reference with data is in total BFI
if ref_bfi not in bfis:
    bfis.append(ref_bfi)
print("brlabel",brlabel)
for bfi in bfis:
    if not Path(bfi).is_file():
        print(f"{bfi} does not exist. Skipping.")
        continue
    if bfi == ref_bfi:
        print("Adding data from bfi",bfi)
    else:
        print("Adding bfi",bfi)
    with open(bfi, "r") as f:
        data = json.load(f)

    for region, contents in data.items():
        # Only save "data" if it contains meaningful, nonzero values, ref_bfi could have different brlabel
        if "data" in contents and bfi == ref_bfi:
            tot_bfi[region]["data"] = contents["data"]
    
        #only save signal yields for BFIs with relevant brlabel
        if brlabel not in bfi:
            continue

        if region not in tot_bfi:
            tot_bfi[region] = {}

        # Save all entries except "data"
        tot_bfi[region].update({
            key: value
            for key, value in contents.items()
            if key != "data"
        })


tot_bfi = sort_nested_dict(tot_bfi)


tot_bfi_name = bfis[0][:bfis[0].find("_Ctau")]
tot_bfi_name += "_CtauScan.json"
with open(tot_bfi_name,"w") as outfile:
    json.dump(tot_bfi, outfile, indent=4)
print(f"Wrote overall ctau scan BFI to {tot_bfi_name}")
