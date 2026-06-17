import json
import re
from pathlib import Path

gluino_xsec = {
        1800 : 4.524E-03,
        1900 : 2.745E-03,
        2000 : 1.684E-03,
        2100 : 1.044e-03,
        2300 : 4.130e-4,
        2500 : 1.687e-4,
        2550 : 1.355e-4,
        2600 : 1.089e-4,
        2650 : 8.772e-5,
        2700 : 7.075e-5
    }
squark_xsec = {
        1800 : 7.357E-04,
        1900 : 4.396E-04,
        2000 : 2.654E-04,
        2100 : 1.618E-04,
        2200 : 9.949E-05,
    }


br_colors = {
    '100PhoBR0ZBR':  'green',
    '50PhoBR50ZBR':  'pink',
    '0PhoBR100ZBR':  'purple',
}
_br_colors_fallback = ['tab:blue', 'tab:orange', 'tab:brown']

def GetMassFromFile(limit_file):
    match_obj = re.search(r"_mGl-\d+_mN2-\d+_mN1-\d+", limit_file)
    mass_pt = []
    if match_obj:
        mass_ptidx = limit_file.find("mGl")
        mass_ptidx_end = re.search(r'mN1-\d+', limit_file).end() 
        mass_pt = limit_file[mass_ptidx:mass_ptidx_end]
        mass_pt = [mass_pt[mass_pt.find("mGl-")+4:mass_pt.find("_mN2")], mass_pt[mass_pt.find("mN2-")+4:mass_pt.find("_mN1")], mass_pt[mass_pt.find("mN1-")+4:]]
        mass_pt = [int(i) for i in mass_pt]
    return mass_pt

def GetMasses( signalString ):
    splitSignal = signalString.split("_")
    mgo = float(splitSignal[0])
    mn2 = float(splitSignal[1])
    mn1 = float(splitSignal[2])
    ctau = float(splitSignal[3])
    return [mgo,mn2,mn1,ctau]

def ReadLimits( inputfilename ):
    f = open(inputfilename, 'r')
    data = json.load(f)

    limit_dict = {}
    sig_label = inputfilename[:inputfilename.find("_limit")]
    for key, val in data.items():
        masses = GetMasses(key)
        #parent mass, mn2, mn1, ctau
        limit_dict[(masses[0],masses[1], masses[2], masses[3])] = val
    #print("label",sig_label)
    return limit_dict, sig_label


def ReadLimitsBRs(inputfiles):
    import re
    br_dict = {}
    for file in inputfiles:
        br_key = Path(file).stem
        br_key = br_key.split("_")[-1]
        m = re.fullmatch(r'Z(\d+)G(\d+)', br_key)
        if m:
            br_key = f"{m.group(2)}PhoBR{m.group(1)}ZBR"
        elif not re.fullmatch(r'\d+PhoBR\d+ZBR', br_key):
            br_key = "50PhoBR50ZBR"
        limit_dict, sig_label = ReadLimits(file)
        br_dict[br_key] = limit_dict
    if list(br_dict.keys())[-1] in sig_label:
        sig_label = sig_label.replace("_"+list(br_dict.keys())[-1],"")
    return br_dict, sig_label

def ReadLimitsKeyword(inputfiles,keyword):
    out_dict = {}
    for file in inputfiles:
        key = Path(file).stem 
        key = [k for k in key.split("_") if keyword in k][0]
        limit_dict, sig_label = ReadLimits(file)
        out_dict[key] = limit_dict
    if list(out_dict.keys())[-1] in sig_label:
        sig_label = sig_label.replace("_"+list(out_dict.keys())[-1],"")
    return out_dict, sig_label


