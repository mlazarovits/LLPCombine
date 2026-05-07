import json
from limit_tools import gluino_xsec


sampleMass = 2300
targetMass = 2100
dMass = sampleMass - targetMass

if targetMass not in gluino_xsec.keys():
    print("Mass",targetMass,"does not have xsec specified")
    exit()
if sampleMass not in gluino_xsec.keys():
    print("Mass",sampleMass,"does not have xsec specified")
    exit()
rwt_factor = gluino_xsec[targetMass] / gluino_xsec[sampleMass]

file = "../json//BigGuy_NonCompressed_FullRegions_SplitSVDelayedPhoton_4BinDelayedPhoton.json"
ofile = f"../json//BigGuy_NonCompressed_FullRegions_SplitSVDelayedPhoton_4BinDelayedPhoton_MassReweight{sampleMass}to{targetMass}.json" 
with open(file,"r") as f:
    data = json.load(f)

rwt_data = {}
for region, procs in data.items():
    rwt_data[region] = {}
    for proc, evts in procs.items():
        if f"gogoGZ_{sampleMass}" in proc:
            rwt_evts = [evts[0], evts[1] * rwt_factor, evts[2] * rwt_factor]
            newproc = proc.replace(str(sampleMass),str(targetMass))
            masses = proc.split("_")
            mN2 = int(masses[2])
            newproc = newproc.replace(str(mN2),str(mN2 - dMass))
            mN1 = int(masses[3])
            newproc = newproc.replace(str(mN1),str(mN1 - dMass))

        else:
            rwt_evts = evts
            newproc = proc
        rwt_data[region][newproc] = rwt_evts
with open(ofile,"w") as f:
    json.dump(rwt_data,f,indent=4)
print("wrote reweighted events to",ofile)
