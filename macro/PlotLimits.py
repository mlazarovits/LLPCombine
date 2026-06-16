import itertools
import math as mt
import matplotlib.pyplot as plt
import numpy as np
#import matplotlib.colors as colors'import matplotlib.cm as cm
import matplotlib.cm as cm
from matplotlib.colors import Normalize, LinearSegmentedColormap
import mplhep as hep
import argparse
import json
from limit_tools import ReadLimits, ReadLimitsBRs, gluino_xsec, br_colors, _br_colors_fallback

hep.style.use("CMS")


def truncate_colormap(cmap, minval=0.2, maxval=0.8, n=256):
    """Truncate a colormap to a narrower range for darker tones."""
    new_cmap = LinearSegmentedColormap.from_list(
        f"trunc({cmap.name},{minval:.2f},{maxval:.2f})", 
        cmap(np.linspace(minval, maxval, n))
    )
    return new_cmap
    


#mS is mass of parent sparticle (ie gluino, squark)
def MakeN2N1dMplot( significance_dict, sig_label, mS=2000, extra_text="", oname="sigs"):
    x,y,z=[],[],[]
    for key in significance_dict:
        x.append(key[1])
        y.append(key[1]-key[2])
        z.append(significance_dict[key])
        
    x=np.array(x)
    print("x",x,"y",y)
    y=np.array(y)
    
    ColMin=0#default to zbi
    ColMax=5
    cmap = plt.cm.viridis
    cmap =truncate_colormap(cmap, 0, 0.8)
    norm = plt.Normalize(vmin=ColMin, vmax=ColMax)
    
    fig, ax = plt.subplots(figsize=(10,8))
    scatter = ax.scatter(x,y,c=z,norm=norm,cmap=cmap,edgecolors='black')
    minmass = 1e6
    for i, txt in enumerate(z):
        v=round(z[i],2)
        color = cmap(norm(v))
        if( y[i] != 1900):
            plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='bottom',color=color,fontweight='bold')
        if( y[i] == 1900):
            plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='top',color=color,fontweight='bold')
        if( x[i] < minmass):
            minmass = x[i]
    #axes[j].scatter(x,y,c=zslice,norm=norm,cmap=cmap,edgecolors='black')
    if("sqsq" in sig_label):
        plt.text(minmass+50,700, "$m_{\\tilde{q}}=$" + str(mS) +" GeV", fontsize=20)
    if("gogo" in sig_label):
        plt.text(minmass+50,700, "$m_{\\tilde{g}}=$" + str(mS) +" GeV", fontsize=20)
    plt.xlabel('$m_{N2}$ (GeV)')
    plt.ylabel('$\Delta(m_{N2},m_{N1})$ (GeV)')

    #plt.yscale('log')
    
    if(extra_text != ""):
        plt.text(minmass+50,800, extra_text, fontsize=20)

    #cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))

    cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
    cbar.set_label("Significance")
    hep.cms.label(rlabel="")
    print("Saving plot as",oname+"_"+sig_label+"_n2n1dM.pdf")
    plt.savefig(oname+"_"+sig_label+"_n2n1dM.pdf")
    #plt.show()

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
    minmass = 1e6
    for i, txt in enumerate(z):
        v=round(z[i],2)
        color = cmap(norm(v))
        if( y[i] != 1900):
            plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='bottom',color=color,fontweight='bold')
        if( y[i] == 1900):
            plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='top',color=color,fontweight='bold')
        if( x[i] < minmass):
            minmass = x[i]
    #axes[j].scatter(x,y,c=zslice,norm=norm,cmap=cmap,edgecolors='black')
    if("sqsq" in sig_label):
        plt.xlabel('$m_{\\tilde{q}}$ (GeV)')
        plt.ylabel('$\Delta(m_{\\tilde{q}},m_{N2})$ (GeV)')
    if("gogo" in sig_label):
        plt.xlabel('$m_{\\tilde{g}}$ (GeV)')
        plt.ylabel('$\Delta(m_{\\tilde{g}},m_{N2})$ (GeV)')

    #plt.yscale('log')
    
    plt.text(minmass+50,700, "$m_{N1}=$" + str(mN1) +" GeV", fontsize=20)
    if(extra_text != ""):
        plt.text(minmass+50,800, extra_text, fontsize=20)

    #cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))

    cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
    cbar.set_label("Significance")
    hep.cms.label(rlabel="")
    print("Saving plot as",oname+"_"+sig_label+"_n2dM.pdf")
    plt.savefig(oname+"_"+sig_label+"_n2dM.pdf")
    #plt.show()

    
def MakeN1N2plot( significance_dict, sig_label, mGo=2000, ctau=10, extra_text="", oname = "sigs"):
    x,y,z = [],[],[]
    for key in significance_dict:
        if ctau != key[3]:
            continue
        if mGo != key[0]:
            continue
        entry = significance_dict[key]
        if "exp0" not in entry:
            print(f"Warning: skipping key={key} — missing 'exp0'")
            continue
        x.append(key[2])#N1 on xaxis
        y.append(key[1])#N2 on yaxis
        #just do median for now * xsec
        z.append(entry["exp0"] * gluino_xsec[mGo] * 1000) #put in fb from pb
    if not x:
        print(f"Warning: no complete entries for mGo={mGo}, ctau={ctau} — skipping N1N2 plot")
        return
    x=np.array(x)
    y=np.array(y)

    xmax = max(x)
    xmin = min(x)
    ymax = max(y)
    ymin = min(y)

    ColMin=min(z)#default to zbi
    ColMax=max(z)
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
    plt.xlabel('$m_{N1}$ (GeV)')
    plt.ylabel('$m_{N2}$ (GeV)')
    #plt.yscale('log')
    plot_sig_label = ""
    if("sqsq" in sig_label):
        plot_sig_label = "$m_{\\tilde{q}}=$" + str(mGo) +" GeV"
    elif("gogo" in sig_label):
        plot_sig_label = "$m_{\\tilde{g}}=$" + str(mGo) +" GeV"
    else:
        plot_sig_label = "$m_{\\tilde{S}}=$" + str(mGo) +" GeV"

    dtext_start = (ymax - ymin)*0.025
    dtext = (ymax - ymin)*0.075
    plt.text(xmin+50,ymax-dtext_start, "$m_{\\tilde{g}}=$" + str(mGo) +" GeV", fontsize=20)
    plt.text(xmin+50,ymax-(dtext_start + dtext), r"c$\tau$ = "+str(ctau)+" cm", fontsize=20)
    if extra_text != "":
        plt.text(xmin+50,ymax-(dtext_start + 2*dtext), extra_text, fontsize=20)
    
    #cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))
    
    cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
    cbar.set_label("Expected 95% CL Cross-Section Upper Limit (fb)")
    hep.cms.label(rlabel="")
    plotname = f"{oname}_{sig_label}_mGl-{mGo}_ctau-{ctau}_n1n2.pdf"
    print("Saving plot as",plotname)
    
    plt.savefig(plotname)
    #plt.show()



def MakeCtauLimit( significance_dict, sig_label, mGo=2000, mN2 = 1500, mN1 = 500, extra_text="", oname = "sigs", plot_min=1e-5, plot_max=1e0, xlim_min=None, xlim_max=None, lumi=200):
    x,y,y1sigup,y2sigup,y1sigdn,y2sigdn = [],[],[],[],[],[]
    for key in significance_dict:
        if mGo != key[0]:
            continue
        if mN2 != key[1]:
            continue
        if mN1 != key[2]:
            continue
        required = {"exp0", "exp+1", "exp+2", "exp-1", "exp-2"}
        entry = significance_dict[key]
        if not required.issubset(entry.keys()):
            missing = required - entry.keys()
            print(f"Warning: skipping ctau={key[3]} — missing keys {missing}")
            continue
        x.append(key[3])#ctau on xaxis
        #just do median for now * xsec (gluino_xsec in pb, convert to fb)
        y.append(entry["exp0"] * gluino_xsec[mGo] * 1000)
        y1sigup.append(entry["exp+1"] * gluino_xsec[mGo] * 1000)
        y2sigup.append(entry["exp+2"] * gluino_xsec[mGo] * 1000)
        y1sigdn.append(entry["exp-1"] * gluino_xsec[mGo] * 1000)
        y2sigdn.append(entry["exp-2"] * gluino_xsec[mGo] * 1000)
    if not x:
        print(f"Warning: skipping plot for mGo={mGo}, mN2={mN2}, mN1={mN1} — no valid ctau points")
        return

    sort_idx = np.argsort(x)
    x=np.array(x)[sort_idx]
    y=np.array(y)[sort_idx]
    y1sigup=np.array(y1sigup)[sort_idx]
    y2sigup=np.array(y2sigup)[sort_idx]
    y1sigdn=np.array(y1sigdn)[sort_idx]
    y2sigdn=np.array(y2sigdn)[sort_idx]

    xmax = xlim_max if xlim_max is not None else max(x)
    xmin = xlim_min if xlim_min is not None else min(x)
    ymax = max(max(y),max(y1sigup),max(y2sigup))
    ymin = min(min(y),min(y1sigdn),min(y2sigdn))

    fig, ax = plt.subplots(figsize=(10,8))
    green = '#228b22'
    yellow = '#ffcc00'
    ax.plot(x, y, color='k', marker='o', label = "Expected", zorder=10)
    plt.fill_between(np.asarray(x), 
                     np.asarray(y1sigup), 
                     np.asarray(y1sigdn), color=green, label = "68% expected", zorder=3)
    plt.fill_between(np.asarray(x),
                    np.asarray(y2sigup),
                    np.asarray(y2sigdn), color=yellow, label = "95% expected")
    xsec_th = gluino_xsec[mGo] * 1000
    ax.axhline(y=xsec_th, color='blue', linestyle='--', linewidth=2, zorder=20,
               label=r'$\tilde{g}\tilde{g}$ production $\sigma_{th}$')
    ax.axhline(y=xsec_th * 6, color='red', linestyle='--', linewidth=2, zorder=20,
               label=r'$\tilde{g}\tilde{g}$ GMSB $\sigma_{th}$')
    hep.cms.label("Preliminary", data = True, lumi=lumi,com=13.6)

    # Style
    plt.legend(loc=1)
    plt.yscale('log')
    plt.xscale('log')
    plt.ylim(plot_min,plot_max)
    plt.xlim(xmin, xmax)
    ax.set_xlabel("c#tau [cm]")
    plt.xlabel(r'$c\tau$ (cm)')
    plt.ylabel('95% CL upper limit on cross section (fb)')
    #plt.yscale('log')
    plot_sig_label = ""
    if("sqsq" in sig_label):
        plot_sig_label = "$m_{\\tilde{q}}=$" + str(mGo) +" GeV"
    elif("gogo" in sig_label):
        plot_sig_label = "$m_{\\tilde{g}}$ = "+str(mGo)+", $m_{\\tilde{\\chi}^0_2}$ = " + str(mN2) + ", $m_{\\tilde{\\chi}^0_1}$ = "+str(mN1)
    else:
        plot_sig_label = "$m_{\\tilde{S}}=$" + str(mGo) +" GeV"

    plt.text(0.98, 0.04, plot_sig_label, fontsize=16, ha='right', va='bottom', transform=ax.transAxes)
    if extra_text != "":
        plt.text(0.98, 0.10, extra_text, fontsize=16, ha='right', va='bottom', transform=ax.transAxes)
    plotname = f"{oname}_{sig_label}_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}_ctau1D.pdf"
    print("Saving plot as",plotname)
    plt.savefig(plotname)

def MakeCtauLimitMultipleBRs( br_dicts, sig_label, mGo=2000, mN2 = 1500, mN1 = 500, extra_text="", oname = "sigs", plot_min=1e-5, plot_max=1e0, xlim_min=None, xlim_max=None):
    fig, ax = plt.subplots(figsize=(10,8))
    br_idx = 0
    global_xmin = float('inf')
    global_xmax = float('-inf')
    for br_key, significance_dict in br_dicts.items():
        x,y,y1sigup,y2sigup,y1sigdn,y2sigdn = [],[],[],[],[],[]
        for key in significance_dict:
            if mGo != key[0]:
                continue
            if mN2 != key[1]:
                continue
            if mN1 != key[2]:
                continue
            required = {"exp0", "exp+1", "exp+2", "exp-1", "exp-2"}
            entry = significance_dict[key]
            if not required.issubset(entry.keys()):
                missing = required - entry.keys()
                print(f"Warning: skipping ctau={key[3]} (br={br_key}) — missing keys {missing}")
                continue
            x.append(key[3])#ctau on xaxis
            #just do median for now * xsec (gluino_xsec in pb, convert to fb)
            y.append(entry["exp0"] * gluino_xsec[mGo] * 1000)
            y1sigup.append(entry["exp+1"] * gluino_xsec[mGo] * 1000)
            y2sigup.append(entry["exp+2"] * gluino_xsec[mGo] * 1000)
            y1sigdn.append(entry["exp-1"] * gluino_xsec[mGo] * 1000)
            y2sigdn.append(entry["exp-2"] * gluino_xsec[mGo] * 1000)
        if not x:
            print(f"Warning: skipping br={br_key} for mGo={mGo}, mN2={mN2}, mN1={mN1} — no valid ctau points")
            continue
        sort_idx = np.argsort(x)
        x=np.array(x)[sort_idx]
        y=np.array(y)[sort_idx]
        y1sigup=np.array(y1sigup)[sort_idx]
        y2sigup=np.array(y2sigup)[sort_idx]
        y1sigdn=np.array(y1sigdn)[sort_idx]
        y2sigdn=np.array(y2sigdn)

        global_xmin = min(global_xmin, min(x))
        global_xmax = max(global_xmax, max(x))

        green = '#228b22'
        yellow = '#ffcc00'
        color = br_colors.get(br_key, _br_colors_fallback[br_idx % len(_br_colors_fallback)])
        ax.plot(x, y, color=color, linestyle='solid', label = br_key, zorder=10)
        br_idx += 1
        #plt.fill_between(np.asarray(x), 
        #                 np.asarray(y1sigup), 
        #                 np.asarray(y1sigdn), color=green, label = "68% expected", zorder=3)
        #plt.fill_between(np.asarray(x),
        #                np.asarray(y2sigup),
        #                np.asarray(y2sigdn), color=yellow, label = "95% expected")
    xsec_th = gluino_xsec[mGo] * 1000
    ax.axhline(y=xsec_th, color='blue', linestyle='--', linewidth=2, zorder=20,
               label=r'$\tilde{g}\tilde{g}$ production $\sigma_{th}$')
    ax.axhline(y=xsec_th * 6, color='red', linestyle='--', linewidth=2, zorder=20,
               label=r'$\tilde{g}\tilde{g}$ GMSB $\sigma_{th}$')
    hep.cms.label("Preliminary", data = True, lumi=lumi,com=13.6)

    # Style
    plt.legend(loc=1)
    plt.yscale('log')
    plt.xscale('log')
    plt.ylim(plot_min,plot_max)
    xmin_plot = xlim_min if xlim_min is not None else (global_xmin if global_xmin != float('inf') else None)
    xmax_plot = xlim_max if xlim_max is not None else (global_xmax if global_xmax != float('-inf') else None)
    if xmin_plot is not None and xmax_plot is not None:
        plt.xlim(xmin_plot, xmax_plot)
    ax.set_xlabel("c#tau [cm]")
    plt.xlabel(r'$c\tau$ (cm)')
    plt.ylabel('95% CL upper limit on cross section (fb)')
    #plt.yscale('log')
    plot_sig_label = ""
    if("sqsq" in sig_label):
        plot_sig_label = "$m_{\\tilde{q}}=$" + str(mGo) +" GeV"
    elif("gogo" in sig_label):
        plot_sig_label = "$m_{\\tilde{g}}$ = "+str(mGo)+", $m_{\\tilde{\\chi}^0_2}$ = " + str(mN2) + ", $m_{\\tilde{\\chi}^0_1}$ = "+str(mN1)
    else:
        plot_sig_label = "$m_{\\tilde{S}}=$" + str(mGo) +" GeV"

    plt.text(0.98, 0.04, plot_sig_label, fontsize=16, ha='right', va='bottom', transform=ax.transAxes)
    if extra_text != "":
        plt.text(0.98, 0.10, extra_text, fontsize=16, ha='right', va='bottom', transform=ax.transAxes)
    plotname = f"{oname}_{sig_label}_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}_ctau1DMultipleBRs.pdf"
    print("Saving plot as",plotname)
    
    plt.savefig(plotname)

parser = argparse.ArgumentParser()
parser.add_argument("--input", "-i", required=True, nargs='+',help="input limit .json file")
parser.add_argument("--lumi",help='luminosity',default='200')
parser.add_argument("--extra", "-e", help="extra text")
parser.add_argument("--mass-point", "-m", nargs=3, type=int, action='append',
                    metavar=('mGo', 'mN2', 'mN1'),
                    help="mass point triplet (mGluino mN2 mN1); can be given multiple times")
parser.add_argument("--ymin", type=float, default=1e-5, help="y-axis lower limit (default: 1e-5)")
parser.add_argument("--ymax", type=float, default=1e0,  help="y-axis upper limit (default: 1e0)")
parser.add_argument("--xmin", type=float, default=None, help="x-axis lower limit (default: auto from data)")
parser.add_argument("--xmax", type=float, default=None, help="x-axis upper limit (default: auto from data)")
args = parser.parse_args()

print("n jsons passed",len(args.input))

extra_text = args.extra
textfile = args.input[0]
ofile = textfile[:textfile.find(".json")]
lumi = args.lumi

default_mass_pts = [[2300, 1300, 1000], [2500, 1200, 500]]
mass_pts = args.mass_point if args.mass_point else default_mass_pts

if len(args.input) > 1:
    #only do multiBR ctau limit for now
    br_dict, sig_label = ReadLimitsBRs(args.input)
    required = {"exp0", "exp+1", "exp+2", "exp-1", "exp-2"}
    all_ctaus = [key[3] for sig_dict in br_dict.values() for key, entry in sig_dict.items() if required.issubset(entry.keys())]
    ctau_xmin = args.xmin if args.xmin is not None else (min(all_ctaus) if all_ctaus else None)
    ctau_xmax = args.xmax if args.xmax is not None else (max(all_ctaus) if all_ctaus else None)
    for mp in mass_pts:
        MakeCtauLimitMultipleBRs( br_dict, sig_label, mp[0], mp[1], mp[2], extra_text, ofile, args.ymin, args.ymax, ctau_xmin, ctau_xmax)
    exit()

significance_dict, sig_label = ReadLimits(textfile)
print("sig",sig_label)
n1mass = []
n2mass = []
smass = []
ctaus = []
for key in significance_dict:
    #print(key, significance_dict[key])
    smass.append(key[0])
    n2mass.append(key[1])
    n1mass.append(key[2])
    ctaus.append(key[3])



#do 1D ctau limits
required_keys = {"exp0", "exp+1", "exp+2", "exp-1", "exp-2"}
valid_ctaus = [key[3] for key, entry in significance_dict.items() if required_keys.issubset(entry.keys())]
ctau_xmin = args.xmin if args.xmin is not None else (min(valid_ctaus) if valid_ctaus else None)
ctau_xmax = args.xmax if args.xmax is not None else (max(valid_ctaus) if valid_ctaus else None)
for mp in mass_pts:
    print("making ctau limit plot for mass pt", mp)
    MakeCtauLimit( significance_dict, sig_label, mp[0], mp[1], mp[2], extra_text, ofile, args.ymin, args.ymax, ctau_xmin, ctau_xmax, lumi)

#if multiple n1 masses
if(len(set(n1mass)) > 1 and len(set(smass)) == 1):
    if(len(np.unique(n2mass)) == len(n2mass)): #all n2s are unique
        print("making N1N2 plot")
        #plot N1N2
        MakeN1N2plot( significance_dict, sig_label, smass[0], extra_text, ofile)
    else: #unique mass splittings
        print("making N2N1dM plot")
        #plot N1N2
        MakeN2N1dMplot( significance_dict, sig_label, smass[0], extra_text, ofile)
elif(len(set(n1mass)) == 1 and len(set(smass)) > 1):
    print("making N2dM plot")
    #plot n2 vs dM 
    MakeSN2dMplot( significance_dict, sig_label, n1mass[0], extra_text, ofile)
elif(len(set(n1mass)) == 1 and len(set(smass)) == 1):
    print("making N1N2 plot")
    #plot N1N2
    MakeN1N2plot( significance_dict, sig_label, smass[0], extra_text, ofile)
    print("making N2dM plot")
    #plot n2 vs dM 
    MakeSN2dMplot( significance_dict, sig_label, n1mass[0], extra_text, ofile)
else:
    ctaus = np.unique(ctaus)
    smasses = np.unique(smass)
    for smass, ctau in list(itertools.product(smasses, ctaus)):
        print("making N1N2 plot for parent sparticle mass",smass,"and ctau",ctau,"cm")
        #plot N1N2
        MakeN1N2plot( significance_dict, sig_label, int(smass), int(ctau), extra_text, ofile)



