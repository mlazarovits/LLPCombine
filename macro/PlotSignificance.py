import ROOT as rt
import math as mt
import matplotlib.pyplot as plt
import numpy as np
#import matplotlib.colors as colors'import matplotlib.cm as cm
import matplotlib.cm as cm
from matplotlib.colors import Normalize, LinearSegmentedColormap
import mplhep as hep
import argparse

hep.style.use("CMS")


def truncate_colormap(cmap, minval=0.2, maxval=0.8, n=256):
    """Truncate a colormap to a narrower range for darker tones."""
    new_cmap = LinearSegmentedColormap.from_list(
        f"trunc({cmap.name},{minval:.2f},{maxval:.2f})", 
        cmap(np.linspace(minval, maxval, n))
    )
    return new_cmap
    

def GetMasses( signalString ):
	
	splitSignal = signalString.split("_")
	mgo = float(splitSignal[1]) 
	mn2 = float(splitSignal[2])
	mn1 = float(splitSignal[3])
	return [mgo,mn2,mn1]


def ReadSignificance( inputfilename ):
	f = open(inputfilename, 'r')
	lines = f.readlines()
	
	significance_dict = {}
	for line in lines:
		#print(line.strip())
		splitLine = line.split(' ')
		masses = GetMasses(splitLine[0])
		significance_dict[(masses[1], masses[2])] = float(splitLine[1])

	return significance_dict
	
def ReadSignificance_3key( inputfilename ):
	f = open(inputfilename, 'r')
	lines = f.readlines()
	
	significance_dict = {}
	sig_label = ""
	for line in lines:
		#print(line.strip())
		if(sig_label == ""):
		    sig_label = line[:line.find("_")]
		splitLine = line.split(' ')
		masses = GetMasses(splitLine[0])
		significance_dict[(masses[0],masses[1], masses[2])] = float(splitLine[1])
	print("label",sig_label)
	return significance_dict, sig_label

def MakeSN2dMplot( significance_dict, sig_label, mN1=100, extra_text="", oname="sigs"):
	x,y,z=[],[],[]
	for key in significance_dict:
		x.append(key[0])
		y.append(key[0]-key[1])
		z.append(significance_dict[key])
		
	x=np.array(x)
	y=np.array(y)
	
	ColMin=0#default to zbi
	ColMax=5
	cmap = plt.cm.viridis
	cmap =truncate_colormap(cmap, 0, 0.8)
	norm = plt.Normalize(vmin=ColMin, vmax=ColMax)
	
	fig, ax = plt.subplots(figsize=(10,8))
	scatter = ax.scatter(x,y,c=z,norm=norm,cmap=cmap,edgecolors='black')
	for i, txt in enumerate(z):
		v=round(z[i],2)
		color = cmap(norm(v))
		if( y[i] != 1900):
			plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='bottom',color=color,fontweight='bold')
		if( y[i] == 1900):
			plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='top',color=color,fontweight='bold')
	#axes[j].scatter(x,y,c=zslice,norm=norm,cmap=cmap,edgecolors='black')
	if(sig_label == "sqsq"):
		plt.xlabel('$m_{\\tilde{q}}$ (GeV)')
		plt.ylabel('$\Delta(m_{\\tilde{q}},m_{N2})$ (GeV)')
	if("gogo" in sig_label):
	    plt.xlabel('$m_{\\tilde{g}}$ (GeV)')
	    plt.ylabel('$\Delta(m_{\\tilde{g}},m_{N2})$ (GeV)')

	#plt.yscale('log')
	
	plt.text(1950,700, "$m_{N1}=$" + str(mN1) +" GeV", fontsize=20)
	plt.text(1950,800, extra_text, fontsize=20)

	#cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))

	cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
	cbar.set_label("Significance")
	hep.cms.label(rlabel="")
	print("Saving plot as",oname+"_"+sig_label+"_n2dM.pdf")
	plt.savefig(sig_label+"_n2dM.pdf")
	#plt.show()

	
def MakeN1N2plot( significance_dict, sig_label, mGo=2000, extra_text="", oname = "sigs"):
	x,y,z = [],[],[]
	for key in significance_dict:
		x.append(key[1])#N2 on xaxis
		y.append(key[0])#N1 on yaxis
		z.append(significance_dict[key])
		
	x=np.array(x)
	y=np.array(y)
	
	ColMin=0#default to zbi
	ColMax=5
	cmap = plt.cm.viridis
	cmap =truncate_colormap(cmap, 0, 0.8)
	norm = plt.Normalize(vmin=ColMin, vmax=ColMax)
	
	fig, ax = plt.subplots(figsize=(10,8))
	scatter = ax.scatter(x,y,c=z,norm=norm,cmap=cmap,edgecolors='black')
	for i, txt in enumerate(z):
		v=round(z[i],2)
		color = cmap(norm(v))
		if( y[i] != 1900):
			plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='bottom',color=color,fontweight='bold')
		if( y[i] == 1900):
			plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='top',color=color,fontweight='bold')
	#axes[j].scatter(x,y,c=zslice,norm=norm,cmap=cmap,edgecolors='black')
	plt.xlabel('$m_{N1}$ (GeV)')
	plt.ylabel('$m_{N2}$ (GeV)')
	#plt.yscale('log')
	if(sig_label == "sqsq"):
	    plt.text(1200,700, "$m_{\\tilde{q}}=$" + str(mGo) +" GeV", fontsize=20)
	if("gogo" in sig_label):
	    plt.text(1200,700, "$m_{\\tilde{g}}=$" + str(mGo) +" GeV", fontsize=20)
	
	plt.text(1200,800, extra_text, fontsize=20)

	#cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))

	cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
	cbar.set_label("Significance")
	hep.cms.label(rlabel="")
	print("Saving plot as",oname+"_"+sig_label+"_n1n2.pdf")

	plt.savefig(oname+"_"+sig_label+"_n1n2.pdf")
	#plt.show()




#textfile = './SignificanceFiles/Significance_allbin.txt'
#textfile = './SignificanceFiles/Significance_11j.txt'
#textfile = './SignificanceFiles/Significance_22j.txt'
#textfile = './SignificanceFiles/Significance_2GLLL.txt'
#textfile = './SignificanceFiles/Significance_sq.txt'
#extra_text ="All 3 bins"

parser = argparse.ArgumentParser()
parser.add_argument("--input", "-i", required=True, help="input significance .txt file")
parser.add_argument("--extra", "-e", help="extra text")
args = parser.parse_args()

extra_text = args.extra
textfile = args.input
ofile = textfile[:textfile.find(".txt")]

#significance_dict = ReadSignificance(textfile)
#for key in significance_dict:
#	print(key, significance_dict[key])
#MakeN1N2plot( significance_dict, 2000, extra_text)

significance_dict, sig_label = ReadSignificance_3key(textfile)
n1mass = []
gmass = []
for key in significance_dict:
	print(key, significance_dict[key])
	n1mass.append(key[2])
	gmass.append(key[0])
#if multiple n1 masses
if(len(set(n1mass)) > 1 and len(set(gmass)) == 1):
    print("making N1N2 plot")
    #plot N1N2
    MakeN1N2plot( significance_dict, sig_label, gmass[0], extra_text, ofile)
if(len(set(n1mass)) == 1 and len(set(gmass)) > 1):
    print("making N2dM plot")
    #plot n2 vs dM 
    MakeSN2dMplot( significance_dict, sig_label, n1mass[0], extra_text, ofile)
if(len(set(n1mass)) == 1 and len(set(gmass)) == 1):
    print("making N1N2 plot")
    #plot N1N2
    MakeN1N2plot( significance_dict, sig_label, gmass[0], extra_text, ofile)
    print("making N2dM plot")
    #plot n2 vs dM 
    MakeSN2dMplot( significance_dict, sig_label, n1mass[0], extra_text, ofile)




