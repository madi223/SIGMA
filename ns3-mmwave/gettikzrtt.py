import numpy as np
#import scipy.stats
import argparse
import time
from datetime import datetime
import sys
import os
import csv
from scipy.stats import t
from scipy import sqrt
from statistics import variance, mean 
def int_ech(values,conf=0.95) :
    n = len(values) 
    m = mean(values) 
    s = variance(values)
    proba = (1-conf)*100 ; proba = (100-proba/2)/100 
    ddl = n - 1
    intervalle = sqrt(s/n) * t.ppf(proba, ddl)
    return m,intervalle
######## ARGS definition ######
parser = argparse.ArgumentParser()
parser.add_argument("--data","-d",help="absolute path of dataset.csv",required=True)
args = parser.parse_args()
##############################
Data = args.data

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0 * np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * scipy.stats.t.ppf((1 + confidence) / 2., n-1)
    return m, h, m-h, m+h


#dataset = pd.read_csv(Data)
i=0
now = datetime.now()
timestamp = datetime.timestamp(now)

f = open("ci-tikz.csv","w")
f.write("conn,duration,error\n")
algo=0
while i < 1:
    g2c = []
    g10c = []
    g20c = []
    g2b = []
    g10b = []
    g20b = []
    tr = open(Data,"r")
    csv_reader = csv.reader(tr, delimiter=',')
    j=0
    algo+=1
    for row in csv_reader:
        if ((j!=0) and ((str(row[0])) == "Cubic.1") and (float(row[9])==1)):
            g2c.append(float(row[10]))
            #j +=1
        if ((j!=0) and ((str(row[0])) == "Cubic.1") and (float(row[9])==2)):
            g2b.append(float(row[10]))
        if ((j!=0) and ((str(row[0])) == "Cubic.1") and (float(row[9])==4)):
            g20c.append(float(row[10]))
        if ((j!=0) and ((str(row[0])) == "Cubic.1") and (float(row[9])==8)):
            g10c.append(float(row[10]))
        if ((j!=0) and ((str(row[0])) == "Cubic.1") and (float(row[9])==16)):
            g20b.append(float(row[10]))
        j+=1
    if len(g2c) >= 2:
        #f.write("{},".format(2))
        m,ci = int_ech(g2c)
        f.write("{},{},{}\n".format(1,m,ci))
    if len(g2b) >=2:
        m,ci = int_ech(g2b)
        f.write("{},{},{}\n".format(2,m,ci))
    if len(g20c) >= 2:
        m,ci = int_ech(g20c)
        f.write("{},{},{}\n".format(4,m,ci))
    if len(g10c) >=2:
        #f.write("{},".format(8))
        m,ci = int_ech(g10c)
        f.write("{},{},{}\n".format(8,m,ci))
    if len(g20b) >=2:
        m,ci = int_ech(g20b)
        f.write("{},{},{}\n".format(16,m,ci))
    tr.close()
    i+=1
f.close()
