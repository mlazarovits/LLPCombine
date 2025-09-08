if [ -z "$1" ]
then
        echo "Please pass [datacard_dir] to make corresponding limits and significances"
        return
fi
#dcdir=datacards
#dcdir=datacards_22j
#dcdir=datacards_11j
#dcdir=datacards_2GLLL
dcdir=$1
echo "Making limts for datacards in" $dcdir
combineTool.py -M AsymptoticLimits  -d ${dcdir}/*/*.txt --there -n .limit --parallel 4
echo "Making significances for datacards in" $dcdir
combineTool.py -M Significance -t -1 --expectSignal=1 -d ${dcdir}/*/*.txt --there --parallel 4
