import pandas as pd
import numpy as np
import argparse
import time
from datetime import datetime
import sys
import os
import csv
from scipy.stats import t
from scipy import sqrt
from statistics import variance, mean
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
plt.rcParams['pdf.fonttype'] = 42

######## ARGS definition ######
parser = argparse.ArgumentParser()
parser.add_argument("--c1","-a",help="absolute path of dataset.csv",required=True)
parser.add_argument("--new",help="draw a barplot",action="store_true")
args = parser.parse_args()
##############################
#matplotlib.rcParams['text.usetex'] = True
#sns.set_context("paper")
#sns.set(style="whitegrid", palette="pastel")
sns.set(style="whitegrid", palette="tab10")
#sns.set_context("paper", font_scale=1.9,rc={"lines.linewidth": 9})
sns.set_context("paper",font_scale=1.9, rc={"lines.linewidth": 6.5})#font_scale=1.5,rc={"lines.linewidth": 2.5})
#sns.set(style="ticks", rc={"lines.linewidth": 2.5})
cubic1 = args.c1
c1data = pd.read_csv(cubic1)
if args.new:
    c1data.columns = ['time','rtt','error']
else:
    c1data.columns = ['time','rtt','error','bw','bwerror']

plt.figure(figsize=(15, 13))
t1=sns.ecdfplot(data=c1data,x="rtt")
t1.tick_params(labelsize=44)
cca = cubic1.split(".")[0]
plt.ylabel('CDF',fontsize=51)
plt.xlabel('Average RTT (ms)',fontsize=51)
plt.legend(labels=[cca],fontsize=40)
plt.savefig('ecdf.png')
plt.savefig('ecdf.pdf')
plt.show()
#dataset.index = np.arange(0, len(dataset))
#dataset.to_csv (output+"-mean.csv", index = True, header=False)
