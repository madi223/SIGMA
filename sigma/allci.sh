current=$(pwd)
if [ $# -lt 1 ]
then
   echo "[usage]: ./allci.sh <logdir>"
   exit 1
fi

logdir=$1

####### RTT in ms ##################
cd $logdir
mkdir temp
#cd $logdir

#### RTT increase at every second ########
cd $current
#cp mean.py $logdir
cp moving-avg-leg.py $logdir
cd $logdir
i=1;
logtmp="./res.tmp"
for f in *.csv; do
    #python3 mean.py -d $f -o "./temp/r"$i;
    cat  $f > $logtmp
    python3 moving-avg-leg.py -d $logtmp -o "./temp/r"$i;
    i=$((i+1 ));
done

#### Combining csv files together #######
cd temp/
i=1
rm -rf resAll.csv
for f in *.csv; do
    cat $f >> resAll.csv;
    i=$((i+1 ));
done

#### Calculate Confidence Interval ######
cd $current
python3 getintpep-leg.py -d $logdir"/temp/resAll.csv"
