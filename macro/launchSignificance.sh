
#dcdir=datacards_test1_MCStats
#dcdir=datacards_2photon_prompt
#dcdir=datacards_2photon_prompt4bin
dcdir=$1
pushd ../
#combineTool.py -M AsymptoticLimits  -d ${dcdir}/*/*.txt --there -n .limit --parallel 4
combineTool.py -M Significance -t -1 --expectSignal=1 -d ${dcdir}/*/*.txt --there --parallel 4
#combineTool.py -M Significance -t -1 --expectSignal=0 -d ${dcdir}/*/*.txt --there --parallel 4
popd
