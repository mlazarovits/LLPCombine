import argparse
import os
import ROOT as rt

def get_directories(path):
    """
    Returns a list of directory names within the specified path.
    """
    directories = []
    try:
        # Get all entries in the directory
        all_entries = os.listdir(path)
        
        # Iterate through entries and check if they are directories
        for entry in all_entries:
            full_path = os.path.join(path, entry) # Construct full path
            if os.path.isdir(full_path):
                directories.append(entry)
    except FileNotFoundError:
        print(f"Error: The path '{path}' was not found.")
    except Exception as e:
        print(f"An error occurred: {e}")
        
    return directories


parser = argparse.ArgumentParser()
parser.add_argument("--datacards","-d",help="directory of datacards",required=True)
parser.add_argument("--output","-o",help="output txt file of collected signficances",default=None)
args = parser.parse_args()

#specify datacard dir

#datacard_dir = "datacards"
#datacard_dir = "../datacards_22j"
#datacard_dir = "../datacards_11j"
#datacard_dir = "../datacards_2GLLL"
#datacard_dir = "../datacards_eos"
#datacard_dir = "../datacards_sq"
#datacard_dir = "../datacards_2photon_prompt4bin"
#datacard_dir = "../datacards_PhoSV_2PhoPrompt_Hadronic_NonCompressed_Sensitivity"
datacard_dir = args.datacards#"../datacards_SV_1Hadronic_PureSV_NonCompressed_Sensitivity"
datacard_subdir_list = get_directories(datacard_dir)

ofile = args.output
if ofile is None:
    ofile = "sigs.txt"

with open(ofile, "w") as file:
    sig=-1;
    for subdir in datacard_subdir_list:
        f = rt.TFile.Open(datacard_dir+"/"+subdir+"/higgsCombine.Test.Significance.mH120.root")
        tree = f.Get("limit")
        for entry in tree:
            sig = entry.limit
        line = subdir +" "+ str(sig) +"\n"
        file.write(line)
    file.close()
print("Wrote significances to",ofile)
