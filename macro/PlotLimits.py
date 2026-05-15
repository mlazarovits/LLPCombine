import ROOT as rt
import itertools
import math as mt
import matplotlib.pyplot as plt
import numpy as np
#import matplotlib.colors as colors'import matplotlib.cm as cm
import matplotlib.cm as cm
from matplotlib.colors import Normalize, LinearSegmentedColormap
from scipy.interpolate import LinearNDInterpolator, griddata
import mplhep as hep
import argparse
import json
from limit_tools import ReadLimits, ReadLimitsBRs, ReadLimitsKeyword, gluino_xsec, br_colors, GetMassFromFile

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

def MakeSN1Limit( significance_dict, sig_label, ctau=10, extra_text="", oname="sigs", comp=False):
    x,y,z=[],[],[]
    msN1_best_limits = {}
    for key in significance_dict:
        if ctau != key[3]:
            continue
        #do compressed + noncompressed points separately
        if comp and (key[0] - key[2]) > 200:
            continue
        if not comp and (key[0] - key[2]) <= 200:
            continue
        cur_lim = significance_dict[key]["exp0"]# * gluino_xsec[key[0]] * 1000
        #print("key",key,"lim",cur_lim)
        if (key[0],key[2]) not in msN1_best_limits.keys():
            msN1_best_limits[(key[0], key[2])] = cur_lim
        if cur_lim < msN1_best_limits[(key[0], key[2])]:
            msN1_best_limits[(key[0], key[2])] = cur_lim

    interp_pts = []
    for key, item in msN1_best_limits.items():
        x.append(key[0]) #mS on x-axis
        y.append(key[1]) #mN1 on y-axis
        interp_pts.append((key[0],key[1]))
        ##just do median for now * xsec
        z.append(item) 
      

    interpolator = LinearNDInterpolator(interp_pts, z)
    gridpts_x = np.arange(min(x),max(x),1)
    gridpts_y = np.arange(min(y),max(y),10)
    X, Y = np.meshgrid(gridpts_x, gridpts_y)
    gridpts = np.vstack([X.ravel(), Y.ravel()]).T
    weights = interpolator(gridpts)

    x=np.array(x)
    y=np.array(y)
   
    xmax = max(x)
    xmin = min(x)
    ymax = max(y)
    ymin = min(y)


    ColMin=min(z)
    ColMax=max(z)
    cmap = plt.cm.viridis
    cmap =truncate_colormap(cmap, 0, 0.8)
    norm = plt.Normalize(vmin=ColMin, vmax=ColMax)
    
    fig, ax = plt.subplots(figsize=(10,8))
    #scatter = ax.scatter(x,y,c=z,norm=norm,cmap=cmap,edgecolors='black')
    ax.scatter(X, Y, c=weights,norm=norm,cmap=cmap)
    ax.plot(x, y, color='pink',marker='*',linestyle="",markersize=30,zorder=4)
    #write numbers on individual points
    #for i, txt in enumerate(z):
    #    v=round(z[i],2)
    #    color = cmap(norm(v))
    #    if( y[i] != 1900):
    #        plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='bottom',color=color,fontweight='bold')
    #    if( y[i] == 1900):
    #        plt.text(x[i],y[i], str(round(z[i],2)),fontsize=14, ha='center', va='top',color=color,fontweight='bold')
    plt.ylabel('$m_{N1}$ (GeV)')
    #plt.yscale('log')
    if("sqsq" in sig_label):
        plt.xlabel('$m_{\\tilde{q}}$ (GeV)')
    elif("gogo" in sig_label):
        plt.xlabel('$m_{\\tilde{g}}$ (GeV)')
    else:
        plt.xlabel('$m_{\\tilde{S}}$ (GeV)')

    dtext_start = (ymax - ymin)*0.025
    dtext = (ymax - ymin)*0.075
    plt.text(xmin+10,ymax-(dtext_start + dtext), r"c$\tau$ = "+str(ctau)+" cm", fontsize=20)
    if extra_text != "":
        plt.text(xmin+10,ymax-(dtext_start + 2*dtext), extra_text, fontsize=20)
    
    #cbar = plt.colorbar(scatter_plot, label='Color Value',cm.ScalarMappable(norm=norm, cmap=cmap))
    
    cbar = fig.colorbar(cm.ScalarMappable(norm=norm, cmap=cmap),ax=ax, orientation='vertical')#, shrink=0.7)
    cbar.set_label("Median Expected 95% CL $\\sigma/\\sigma_{th}$",size=20)
    hep.cms.label(rlabel="")
    hep.cms.label(rlabel="")
    plotname = f"{oname}_{sig_label}_ctau-{ctau}_mSn1.pdf"
    print("Saving plot as",plotname)
    plt.savefig(plotname)
    #plt.show()
    
def MakeN1N2plot( significance_dict, sig_label, mGo=2000, ctau=10, extra_text="", oname = "sigs"):
    x,y,z = [],[],[]
    interp_pts = []
    interp_vals = []
    for key in significance_dict:
        if ctau != key[3]:
            continue
        if mGo != key[0]:
            continue
        x.append(key[2])#N1 on xaxis
        y.append(key[1])#N2 on yaxis
        interp_pts.append((key[2],key[1]))
        #just do median for now * xsec
        z.append(significance_dict[key]["exp0"] * gluino_xsec[mGo] * 1000) #put in fb from pb
    print("interp_pts",interp_pts)
    print("interp_vals",interp_vals)

    #hist_x=np.arange(min(x), max(x), 10)
    #hist_y=np.arange(min(y), max(y), 10)
    #pts = [pt for pt in zip(hist_x, hist_y)]
    #weights = griddata(interp_pts, interp_vals, pts)
    #for w_pt in zip(weights,pts):
    #    print(w_pt)
    ##weights = [interpolator(pt) for pt in zip(hist_x, hist_y)]
    #test_idx = 20
    #print("weights",weights[test_idx],"pt",hist_x[test_idx],",",hist_y[test_idx])

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
    #scatter = ax.scatter(x,y,c=z,norm=norm,cmap=cmap,edgecolors='black')

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



def MakeCtauLimit( significance_dict, sig_label, mGo=2000, mN2 = 1500, mN1 = 500, extra_text="", oname = "sigs"):
    sparticle_label = ""
    if("sqsq" in sig_label):
        sparticle_label = "\\tilde{q}"
    elif("gogo" in sig_label):
        sparticle_label = "\\tilde{g}"
    else:
        sparticle_label = "\\tilde{S}"
    x,y,y1sigup,y2sigup,y1sigdn,y2sigdn = [],[],[],[],[],[]
    significance_dict = dict(sorted(significance_dict.items()))
    for key in significance_dict:
        if mGo != key[0]:
            continue
        if mN2 != key[1]:
            continue
        if mN1 != key[2]:
            continue
        x.append(key[3])#ctau on xaxis
        #just do median for now * xsec
        y.append(significance_dict[key]["exp0"] * gluino_xsec[mGo] * 1000)
        y1sigup.append(significance_dict[key]["exp+1"] * gluino_xsec[mGo] * 1000)
        y2sigup.append(significance_dict[key]["exp+2"] * gluino_xsec[mGo] * 1000)
        y1sigdn.append(significance_dict[key]["exp-1"] * gluino_xsec[mGo] * 1000)
        y2sigdn.append(significance_dict[key]["exp-2"] * gluino_xsec[mGo] * 1000)
        #print("x",key[3],"med",significance_dict[key]["exp0"] * gluino_xsec[mGo], "1sigup", significance_dict[key]["exp+1"] * gluino_xsec[mGo] * 1000)
    x=np.array(x)
    y=np.array(y)
    y1sigup=np.array(y1sigup)
    y2sigup=np.array(y2sigup)
    y1sigdn=np.array(y1sigdn)
    y2sigdn=np.array(y2sigdn)
    
    xmax = max(x)
    xmin = min(x)
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
    #plot theory line
    x_th = np.arange(xmin, xmax, 100)
    sig_xsecs = {}
    if("gogo" in sig_label):
        sig_xsecs = gluino_xsec
    y_th = np.array([sig_xsecs[mGo]*1000 for i in x])
    y_gmsb = np.array([sig_xsecs[mGo]*1000*6 for i in x])
    ax.plot(x, y_th, color='b', linestyle='dashed',label="$"+sparticle_label+sparticle_label+"$ production $\\sigma_{th}$",zorder=10)
    ax.plot(x, y_gmsb, color='r', linestyle='dashed',label="$"+sparticle_label+sparticle_label+"$ GMSB $\\sigma_{th}$",zorder=9)
    hep.cms.label("Preliminary", data = True, lumi=200,com=13.6)
    
    # Style
    plt.legend(loc=1)
    plt.yscale('log')
    plot_max = 5e2
    plot_min = 1e-2
    plt.ylim(plot_min,plot_max)
    ax.set_xlabel("c#tau [cm]")
    plt.xlabel(r'$c\tau$ (cm)')
    plt.ylabel('95% CL upper limit on cross-section (fb)')
    #plt.yscale('log')
    plot_sig_label = "$m_{"+sparticle_label+"}$ = "+str(mGo)+", $m_{\\tilde{\\chi}^0_2}$ = " + str(mN2) + ", $m_{\\tilde{\\chi}^0_1}$ = "+str(mN1)+" GeV"

    dtext_start =  5e-5 
    dtext = 3e-2
    dtext_xoffset = 0
    plt.text(xmin+dtext_xoffset,plot_max*dtext_start, plot_sig_label, fontsize=20)
    if extra_text != "":
        plt.text(xmin+dtext_xoffset,plot_max*(dtext), extra_text, fontsize=20)
    plotname = oname
    if sig_label not in plotname:
        plotname += "_"+sig_label
    if f"_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}" not in plotname:
        plotname += f"_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}"
    plotname += "_ctau1D.pdf"
    print("Saving plot as",plotname)
    plt.savefig(plotname)

def MakeCtauLimitMultipleBRs( br_dicts, sig_label, mGo=2000, mN2 = 1500, mN1 = 500, extra_text="", oname = "sigs"):
    sparticle_label = ""
    if("sqsq" in sig_label):
        sparticle_label = "\\tilde{q}"
    elif("gogo" in sig_label):
        sparticle_label = "\\tilde{g}"
    else:
        sparticle_label = "\\tilde{S}"
    fig, ax = plt.subplots(figsize=(10,8))
    br_idx = 0
    for br_key, significance_dict in br_dicts.items():
        x,y,y1sigup,y2sigup,y1sigdn,y2sigdn = [],[],[],[],[],[]
        significance_dict = dict(sorted(significance_dict.items()))
        for key in significance_dict:
            if mGo != key[0]:
                continue
            if mN2 != key[1]:
                continue
            if mN1 != key[2]:
                continue
            x.append(key[3])#ctau on xaxis
            #just do median for now * xsec
            y.append(significance_dict[key]["exp0"] * gluino_xsec[mGo] * 1000)
            y1sigup.append(significance_dict[key]["exp+1"] * gluino_xsec[mGo] * 1000)
            y2sigup.append(significance_dict[key]["exp+2"] * gluino_xsec[mGo] * 1000)
            y1sigdn.append(significance_dict[key]["exp-1"] * gluino_xsec[mGo] * 1000)
            y2sigdn.append(significance_dict[key]["exp-2"] * gluino_xsec[mGo] * 1000)
        x=np.array(x)
        y=np.array(y)
        y1sigup=np.array(y1sigup)
        y2sigup=np.array(y2sigup)
        y1sigdn=np.array(y1sigdn)
        y2sigdn=np.array(y2sigdn)
        
        xmax = max(x)
        xmin = min(x)
        ymax = max(max(y),max(y1sigup),max(y2sigup))
        ymin = min(min(y),min(y1sigdn),min(y2sigdn))

        green = '#228b22' 
        yellow = '#ffcc00'
        if br_key in br_colors.keys():
            color = br_colors[br_key]
        else:
            if "maskPho" in br_key:
                color = br_colors["0PhoBR100ZBR"]
            elif "maskSV" in br_key:
                color = br_colors["100PhoBR0ZBR"]
            else:
                color = br_colors["50PhoBR50ZBR"]
        ax.plot(x, y, color=color, label = br_key, zorder=10)
        br_idx += 1
        #plt.fill_between(np.asarray(x), 
        #                 np.asarray(y1sigup), 
        #                 np.asarray(y1sigdn), color=green, label = "68% expected", zorder=3)
        #plt.fill_between(np.asarray(x),
        #                np.asarray(y2sigup),
        #                np.asarray(y2sigdn), color=yellow, label = "95% expected")
    hep.cms.label("Preliminary", data = True, lumi=lumi,com=13.6)
    
    #plot theory line
    x_th = np.arange(xmin, xmax, 100)
    sig_xsecs = {}
    if("gogo" in sig_label):
        sig_xsecs = gluino_xsec
    y_th = np.array([sig_xsecs[mGo]*1000 for i in x])
    y_gmsb = np.array([sig_xsecs[mGo]*1000*6 for i in x])
    ax.plot(x, y_th, color='b', linestyle='dashed',label="$"+sparticle_label+sparticle_label+"$ production $\\sigma_{th}$",zorder=10)
    ax.plot(x, y_gmsb, color='r', linestyle='dashed',label="$"+sparticle_label+sparticle_label+"$ GMSB $\\sigma_{th}$",zorder=9)

    # Style
    plt.legend(loc=1)
    plt.yscale('log')
    plt.xscale('log')
    plot_max = 5e2
    plot_min = 5e-2
    plt.ylim(plot_min,plot_max)
    ax.set_xlabel("c#tau [cm]")
    plt.xlabel(r'$c\tau$ (cm)')
    plt.ylabel('95% CL upper limit on cross section (fb)')
    #plt.yscale('log')
    plot_sig_label = ""
    plot_sig_label = "$m_{"+sparticle_label+"}$ = "+str(mGo)+", $m_{\\tilde{\\chi}^0_2}$ = " + str(mN2) + ", $m_{\\tilde{\\chi}^0_1}$ = "+str(mN1)+" GeV"

    dtext_start =  2e-2 
    dtext = 3e-2
    dtext_xoffset = 0
    plt.text(xmin+dtext_xoffset,plot_max*dtext_start, plot_sig_label, fontsize=20)
    if extra_text != "":
        plt.text(xmin+dtext_xoffset,plot_max*(dtext), extra_text, fontsize=20)
    plotname = oname
    print("plotname",plotname,"sig_label",sig_label)
    if sig_label not in plotname:
        plotname += "_"+sig_label
    if f"_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}" not in plotname:
        plotname += f"_mGl-{mGo}_mN2-{mN2}_mN1-{mN1}"
    plotname += "_ctau1DMultipleBRs.pdf"
    print("Saving plot as",plotname)
    
    plt.savefig(plotname)

parser = argparse.ArgumentParser()
parser.add_argument("--input", "-i", required=True, nargs='+',help="input limit .json file")
parser.add_argument("--lumi",help='luminosity',default='200')
parser.add_argument("--extra", "-e", help="extra text")

args = parser.parse_args()

print("n jsons passed",len(args.input))

extra_text = args.extra
textfile = args.input[0]
ofile = textfile[:textfile.find(".json")]
lumi = args.lumi

if len(args.input) > 1:
    if "_mask" in args.input[0]:
        sig_dict, sig_label = ReadLimitsKeyword(args.input,"mask")
        mass_pt = GetMassFromFile(args.input[0])
        for file in args.input:
            if any([f"-{i}" not in file for i in mass_pt]):
                print("file",file,"does not have mass pt",mass_pt,"all files need to have same mass point. not making plot.")
                exit()
        #remove first BR from outfile name (matches what's done in ReadLimitsBRs)
        ofile = ofile.replace("_"+list(sig_dict.keys())[0],"")
        MakeCtauLimitMultipleBRs( sig_dict, sig_label, mass_pt[0], mass_pt[1], mass_pt[2], extra_text, ofile+"_maskChs")
        exit()
    else:
        br_dict, sig_label = ReadLimitsBRs(args.input)
        mass_pt = GetMassFromFile(args.input[0])
        for file in args.input:
            if any([f"-{i}" not in file for i in mass_pt]):
                print("file",file,"does not have mass pt",mass_pt,"all files need to have same mass point. not making plot.")
                exit()
        #remove first BR from outfile name (matches what's done in ReadLimitsBRs)
        ofile = ofile.replace("_"+list(br_dict.keys())[0],"")
        MakeCtauLimitMultipleBRs( br_dict, sig_label, mass_pt[0], mass_pt[1], mass_pt[2], extra_text, ofile)
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
for limit_file in args.input:
    mass_pt = GetMassFromFile(limit_file)
    if len(mass_pt) > 1:
        print("Making 1D ctau limit for mass",mass_pt)
        MakeCtauLimit( significance_dict, sig_label, mass_pt[0], mass_pt[1], mass_pt[2], extra_text, ofile)
    exit()

#do 2D limit plots
#TODO - wait until we have more mass points for a more fleshed out mass plane
#MakeSN1Limit( significance_dict, sig_label, 10, extra_text, ofile)
#MakeSN1Limit( significance_dict, sig_label, 50, extra_text, ofile)
#exit()
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


