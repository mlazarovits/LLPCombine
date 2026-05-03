import json
from pathlib import Path

gluino_xsec = {
        2300 : 4.134e-4,
        2500 : 1
    }

br_colors = ['green', 'blue', 'pink']

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
    br_dict = {}
    for file in inputfiles:
        br_key = Path(file).stem 
        br_key = br_key.split("_")[-1]
        limit_dict, sig_label = ReadLimits(file)
        br_dict[br_key] = limit_dict
    return br_dict, sig_label


