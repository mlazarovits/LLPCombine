import json
from pathlib import Path

gluino_xsec = {
        2300 : 4.134e-4,   # pb, NLO+NLL
        2500 : 1.687e-4    # pb, NLO+NLL
    }

br_colors = {
    '100PhoBR0ZBR':  'green',
    '50PhoBR50ZBR':  'pink',
    '0PhoBR100ZBR':  'purple',
}
_br_colors_fallback = ['tab:blue', 'tab:orange', 'tab:brown']

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
    print("label",sig_label)
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
    return br_dict, sig_label


