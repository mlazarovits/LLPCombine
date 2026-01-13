
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

#dcdir=datacards_9binData
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9bin --there

#dcdir=datacards_9binData
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9bin --there

#dcdir=datacards_9binData_bficonfig
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9binConfig --there

#dcdir=datacards_9bin3ch
#combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
#combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9bin3ch --there

dcdir=datacards_3ch_cleaned_v43
combineTool.py -M T2W -i ${dcdir}/*/*.txt -o ws.root
combineTool.py -M FitDiagnostics --saveShapes --saveWithUncertainties -d ${dcdir}/*/ws.root -n .Data9bin3chv43cleaned --there

popd

exit
#andres macro here
#!/bin/bash

# Check if datacard directory was provided as argument
#if [ $# -eq 0 ]; then
    echo "Usage: $0 <datacard_directory>"
    echo "Example: $0 datacards_eos"
    exit 1
fi

dcdir=$1

# Check if the datacard directory exists
if [ ! -d "${dcdir}" ]; then
    echo "Error: Datacard directory '${dcdir}' not found"
    exit 1
fi

# Check if there are any signal subdirectories with .txt files
if ! ls ${dcdir}/*/*.txt 1> /dev/null 2>&1; then
    echo "Error: No datacard files (*.txt) found in ${dcdir}/*/."
    echo "Make sure datacards were generated properly."
    exit 1
fi

echo "Running Combine analysis on datacards in: ${dcdir}"
echo "Found signal directories:"
ls -d ${dcdir}/*/

# Process each signal directory individually
for signal_dir in ${dcdir}/*/; do
    if [ -d "$signal_dir" ]; then
        signal_name=$(basename "$signal_dir")
        echo "Processing signal: $signal_name"
        
        # Check if there are datacard files in this directory
        if ls "$signal_dir"*.txt 1> /dev/null 2>&1; then
            echo "  Running AsymptoticLimits for $signal_name..."
            pushd "$signal_dir" > /dev/null
            combine -M AsymptoticLimits -d *.txt -n .limit
            popd > /dev/null
            
            echo "  Running Significance test for $signal_name..."
            pushd "$signal_dir" > /dev/null
            combine -t -1 --expectSignal=1 -M Significance -d *.txt -n .Test
            popd > /dev/null
        else
            echo "  Warning: No datacard files found in $signal_dir"
        fi
    fi
done

echo "Combine analysis completed!"
echo "Results should be in the individual signal directories under ${dcdir}/"

