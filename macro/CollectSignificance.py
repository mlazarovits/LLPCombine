
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
#specify datacard dir

#datacard_dir = "datacards"
#datacard_dir = "../datacards_22j"
#datacard_dir = "../datacards_11j"
#datacard_dir = "../datacards_2GLLL"
#datacard_dir = "../datacards_eos"
datacard_dir = "../datacards_sq"
datacard_subdir_list = get_directories(datacard_dir)


#print(datacard_subdir_list)

with open("../output/Significance_sq.txt", "w") as file:
    sig=-1;
    for subdir in datacard_subdir_list:
        f = rt.TFile.Open(datacard_dir+"/"+subdir+"/higgsCombine.Test.Significance.mH120.root")
        tree = f.Get("limit")
        for entry in tree:
            sig = entry.limit
        line = subdir +" "+ str(sig) +"\n"
        file.write(line)
    file.close()

