import argparse
import os
import ROOT as rt

def get_directories(path, sigs = []):
    """
    Returns a list of directory names within the specified path.
    """
    directories = []
    try:
        # Get all entries in the directory
        if(len(sigs) == 0):
            all_entries = os.listdir(path)
        else:
            all_entries = sigs
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
#specify datacard dir

#datacard_dir = "../datacards_eos"
#sig_list = ["gogoG_2000_1950_1900_10","gogoG_2000_1950_1_10","gogoG_2000_500_250_10","gogoG_1500_500_100_10","gogoG_2500_1500_1000_10"]
#sig_list = ["gogoG_2000_1000_500_10","gogoG_2000_1500_500_10","gogoG_2000_1900_500_10","gogoG_2000_1950_500_10"]
sig_list = ["gogoG_2500_1500_1000_10","gogoG_2000_1950_1000_10","gogoG_2000_1900_1000_10","gogoG_2000_1500_1000_10"]
parser = argparse.ArgumentParser()
parser.add_argument("--directory", "-d", required=True, help="datacard dir with signficance root files")
parser.add_argument("--output", "-o", default="Significances", help="name of output txt file with significances")
parser.add_argument("--sigs", "-s", default=sig_list, help="signal points to  significances",nargs="+")

args = parser.parse_args()

datacard_subdir_list = get_directories(args.directory, args.sigs)

#print(datacard_subdir_list)
ofile = "output/"+args.output+".txt"

with open(ofile, "w") as file:
    sig=-1;
    for subdir in datacard_subdir_list:
        f = rt.TFile.Open(args.directory+"/"+subdir+"/higgsCombine.Test.Significance.mH120.root")
        tree = f.Get("limit")
        for entry in tree:
            sig = entry.limit
        line = subdir +" "+ str(sig) +"\n"
        file.write(line)
    file.close()
print("Wrote significances to",ofile)
