#!/bin/bash

if [ $# -lt 7 ]
then
   echo "[usage]: ./start-sigma.sh <run1,runN> <simTime> <data> <stream> <buff> <rtt> <numUE>"
   exit 1
fi

scen="SIGMA"
#cca=$2
runcsv=$1
simTime=$2
data=$3
stream=$4
buff=$5
rtt=$6
numUE=$7
runlist=()
rmax=$#
rfirst=$((rmax-3));
tmpfile="res.tmp"
tcplog="clean-tcp.csv"
delaylog="clean-e2e.csv"
logdir=$scen"-results/"
trace=$logdir"scen"$scen"-All."$buff"."$data"."$stream".csv"
WORK=$(pwd)
cd $WORK
mkdir $logdir 2>/dev/null
rm -rf clean-*

if [ ! -e $$trace ]; then
    echo -e "stream,scen,cca,run,data,duration,goodput,buffer,rtt,conn,nbrRTT" > $trace
fi
#str=
delimiter=,
s=$runcsv$delimiter
array=();
while [[ $s ]]; do
    array+=( "${s%%"$delimiter"*}" );
    s=${s#*"$delimiter"};
done;

#comm="NS_GLOBAL_VALUE=\"RngRun=" 
comm=" ./waf --run \"ul_sigma200Mhz_5g --simTime="$simTime" --data="$data" --stream="$stream" --buff="$buff" --serverDelay="$rtt" --numUE="$numUE" --run="

rmfiles=$logdir"scen-"$scen"-Run*stream-*."$buff"."$data"."$rtt"."$stream".csv"
rm -rf $rmfiles
for t in ${array[@]}; do
    echo "********************* 5G UL ["$scen"] / Run["$t"] ************************************"
    echo "--------------------------------------------------------------------------------------"
    comm2=$comm""$t"\" 2>&1 | tee "$tmpfile;
    echo $comm2;
    rm -rf $tcplog;
    rm -rf $tmpfile;
    eval "$comm2";   #2>&1 | tee tmp-res;
    newlog=$logdir"scen-"$scen"-Run"$t"."$buff"."$data"."$rtt"."$stream".csv"
    e2elog=$logdir"e2e-scen-"$scen"-"$cca"Run"$t".csv"
    port="1233"
    port2="49152"
    END=$tream
    j="1"
    for i in $(seq $stream); do
      echo $i; 
      streamid=$(echo $port + $i | bc);
      endport=$(echo $port2 + $i | bc);
      streamlog=$logdir"scen-"$scen"-Run"$t"stream-"$i"."$buff"."$data"."$rtt"."$stream".csv"
      streamid=$streamid","$endport; #49153";
      grep $streamid $tcplog > $streamlog;
      ConnStart=$(head -n 1 $streamlog | awk -F , '{print $1}');
      ConnEnd=$(tail -n 1 $streamlog | awk -F , '{print $1}');
      duration=$(echo $ConnEnd - $ConnStart | bc);
      k="1";
      num=$(echo $i - $j | bc);
      echo "num ="$num
      mline=$(grep -n -i "stream \["$num"\]" $tmpfile | awk -F : '{print $1}');
      numf=$(echo $mline + $k | bc);
      echo "numf ="$numf
      Mbytes=$(head -n $numf $tmpfile | tail -n 1 | awk -F : '{print $2}');
      echo "Mbytes ="$Mbytes
      rate=$(echo $Mbytes*8 / $duration | bc);
      nbRTT=$(echo $duration*1000 / $rtt | bc);
      flowID=$scen"."$i
      nbRTT=$(echo $duration*1000 / $rtt | bc);
      line=$flowID","$scen"."$buff"MB."$$rtt","$scen","$t","$Mbytes","$duration","$rate","$buff","$rtt","$stream","$nbRTT;
      echo -e $line >> $trace;
      #mv $tcplog $newlog;
          
    done
    mv $tcplog $newlog;
    #sudo mv $delaylog $e2elog

done

echo "********************* Processing Experiment Data  ************************************"
python3 gettikz_avr_bw.py -d $trace --cca $scen --size $data
logdirr="RTT-"$cca"-"$data"MB/"
mkdir $logdirr
rmfile=$logdirr"*"
rm -rf $rmfile
allflows=$logdir"scen-"$scen"-Run*stream-*."$buff"."$data"."$rtt"."$stream".csv"
#cp $logdir"scen-"$scen"-Run"$t"stream-"$i"."$buff"."$data"."$rtt"."$stream".csv"
cp $allflows $logdirr #reno-results/scen-reno-Run*stream-*.10.10.4.csv
./allci.sh $logdirr
RTTfile=$scen".RTT.95th.csv"
mv RTT.ci.csv $RTTfile
echo -e "--------------------------------------------------------------------------------------\n"
echo "0) Raw experiment data  stored in dir: ["$logdir"]"
echo "1) Goodput (flow duration) with 95th CI stored in : ["$scen".goodput.95th.csv]"
echo "2) EWMA (window=50) of RTT increase with 95th CI: ["$scen".RTT.95th.csv]"
echo "--------------------------------------------------------------------------------------"

python3 cdf1.py -a $RTTfile --new
#python3 gettikz_avr_bw.py -d $trace --cca $scen --size $data
