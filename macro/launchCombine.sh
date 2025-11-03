
#dcdir=datacards
#dcdir=datacards_22j
#dcdir=datacards_11j
#dcdir=datacards_2GLLL
#dcdir=datacards_eos
#dcdir=datacards_sq
#dcdir=datacards_abcd
#dcdir=datacards_test1
dcdir=datacards_test1_MCStats

pushd ../
#combineTool.py -M AsymptoticLimits  -d ${dcdir}/*/*.txt --there -n .limit --parallel 4
#combineTool.py -M Significance -t -1 --expectSignal=1 -d ${dcdir}/*/*.txt --there --parallel 4
#combineTool.py -M Significance -t -1 --expectSignal=0 -d ${dcdir}/*/*.txt --there --parallel 4

#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root --there

#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root --there


#datacard-2-template-analysis_autoMCstat.txt
#datacard-2-template-analysis_bbbstat.txt
#datacard-2-template-analysis.txt

#combineTool.py -M T2W -i ${dcdir}/*/datacard-2-template-analysis.txt -o ws1.root
#combineTool.py -M T2W -i ${dcdir}/*/datacard-2-template-analysis_autoMCstat.txt -o wsMCstat.root
#combineTool.py -M T2W -i ${dcdir}/*/datacard-2-template-analysis_bbbstat.txt -o wsbbb.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws1.root -n .baseFit --there
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/wsMCstat.root -n .MCStat --there
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/wsbbb.root -n .bbb --there

#dcdir=datacards_pseudoshape1
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .baseFitPseudoShape --there

#dcdir=datacards_pseudoshape2bbb
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .bbbFitPseudoShape --there

#dcdir=datacards_pseudoshape3automc
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .autoMCFitPseudoShape --there

#dcdir=datacards_pseudoshape4automconly
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .MConlyFitPseudoShape --there

#dcdir=datacards_pseudoshape4mclnN
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .MClnNFitPseudoShape --there

#dcdir=datacards_pseudoshape5mclnNupdn
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .MClnUpDnNFitPseudoShape --there

#dcdir=datacards_9binMC
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .MC9bin --there

dcdir=datacards_9binData
combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9bin --there


popd
