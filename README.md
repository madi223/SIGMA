# SIGMA

SIGMA (Simple Increase in Goodput based on MEC Awareness) is a simplistic uplink-oriented Congestion Control Algorithm (CCA) that takes advantage of the known location of the bottleneck (which is the RAN in case of MEC and most CDN scenarios) and the availability of radio information at the UE in order to enhance the uplink TCP performance. At a high level, SIGMA relies on 3 mechanisms or states. It first starts with a mechanism known as Max Start, which allows the SIGMA's sender to
start directly sending at the maximum reachable rate, as opposed to the traditional Slow Start. The Max Start phase lasts 1 RTT and after that the SIGMA's sender oscillates between 2 states : 1) The Proportional Adjustment state, in which the CWND is adjusted proportionally to the ratio of lastRTT over minRTT; 2) The Safe Increase State, during which the released/unused radio BW is discover and used  without creating an excessive buffering in the proces. These machanisms make it possible to maintain a full radio link utilization while avoiding on-device bufferbloat, and this since the very first RTT. As such, SIGMA tremendously improves PLTs and the performance of short-lived flows (which constitute the majority of today's Internet flows)  
<img src="sigma-testbed-3.png" alt="SIGMA ns-3 testbed"/>
<br/>

# 1. Building the simulation environment

## 1.1 Legacy ns3 with mmWave

The commands below build ns3 with mmWave module and Full-duplex CSMA support. All the simulations that don't involve SIGMA should be run on this legacy version. 

```

cd ns3-mmwave/
./waf clean
CXXFLAGS="-Wall" ./waf configure --build-profile=optimized
./waf build

```

## 1.2 Modified ns3 that includes SIGMA

The commands below build a modified ns3 with mmWave and SIGMA support. The simulations that require SIGMA should be run on this modified version.

```
cd sigma/
./waf clean
CXXFLAGS="-Wall" ./waf configure --build-profile=optimized
./waf build

```
# 2. Reproducing the results shown in the paper: Upload duration and RTT
## 2.1 Legacy: NewReno, Cubic, BBR
### Step 1: launch the simulation
From the root repository (SIGMA), go to the legacy ns3 directory (ns3-mmwave) and launch the script **"start-experiment.sh"** with the required parameters (cca, runlist, simTime, data, number_of_stream, buff, rtt, number_of_UE). The cca parameter represents the congestion control algorithm you want to evaluate. You have the choice between  NewReno, Cubic or BBR.

```
$cd ns3-mmwave/
$./start-experiment.sh -h
[usage]: ./start-los.sh <cca>  <run1,runn> <simTime> <data> <stream> <buff> <rtt> <numUE>
$./start-los.sh NewReno 1,2,3,4,5,6,7,8,9,10 1 12 1 10 10 1

```
As soon as the script finishes running, the CDF of the average RTTs will be displayed. Two CSV files with the name **<cca>.goodput.95th.csv** and **<cca>.RTT.95th.csv** will be created in the current directory. These two files contains the uploads durations and the raw average RTTs (used to draw the CDF), respectively.
Repeat the same process for Cubic and BBR while changing the parameters of the script to suit the scenario you want to reproduce.

## 2.2 Evaluating SIGMA
### Step 1: launch the simulation
From the root repository (SIGMA), go to the sigma directory (sigma) and launch the script 
**"start-sigma.sh"** with the required parameters (runlist, simTime, data, number_of_stream, buff, rtt, number_of_UE).

```
$cd ns3-mmwave/ 
$./start-experiment.sh -h 
[usage]: ./start-los.sh <cca>  <run1,runn> <simTime> <data> <stream> <buff> <rtt> <numUE> 
$./start-los.sh NewReno 1,2,3,4,5,6,7,8,9,10 1 12 1 10 10 1

``` 
As soon as the script finishes running, the CDF of the average RTTs will be displayed. Two CSV files with the name **SIGMA.goodput.95th.csv** and **SIGMA.RTT.95th.csv** will be created in the current
directory. These two files contains the uploads durations and the raw average RTTs (used to draw the
 CDF), respectively. Repeat the same proces for each SIGMA scenario shown in the paper.

File Size     | Correpondance (for the script)
------------- | -------------
BDP           | 1.2
2BDP          | 2.5
<br/>
