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
######## ARGS definition ######
parser = argparse.ArgumentParser()
parser.add_argument("--data","-d",help="absolute path of dataset.csv",required=True)
parser.add_argument("--output","-o",help="absolute path of key.csv",required=True)
args = parser.parse_args()
##############################
Data = args.data
dataset = pd.read_csv(Data)
dataset.columns = ['time','cwnd','rtt','throughput','ranBw','tbs','inflight','state','dst','src']
#dataset.columns = ['time','port','rtt','srtt','rttmin','cwnd','inflight']
#dataset.columns = ['time','rtt','cwnd']
output = args.output
tab = []
f = open(output+"-mean.csv","w")
'''
product = {'month' : [1,2,3,4,5,6,7,8,9,10,11,12],'demand':[290,260,288,300,310,303,329,340,316,330,308,310]}
df = pd.DataFrame(product)
print (df)
#df.head()

df['pandas_SMA_3'] = df.iloc[:,0].rolling(window=3).mean()
df.head()
df['EMA'] = df['demand'].ewm(span=3,adjust=False).mean()
df.head()
'''
# dropping ALL duplicte values 
dataset.drop_duplicates(subset ="time", 
                     keep = 'last', inplace = True)

 
dataset['SMA_50'] = dataset['rtt'].ewm(span=50,adjust=False).mean()
#dataset['rtt_200'] = dataset['rtt'].expanding(min_periods=50).mean()
#dataset['bw_100'] = dataset['inflight'].expanding(min_periods=50).mean()
#dataset['bw_200'] = dataset['inflight'].ewm(span=100,adjust=False).mean()
df = dataset.iloc[::50]
df.head()
#df.reset_index()

df.iloc[0,10] = df.iloc[0,2]
#df.iloc[0,11] = df.iloc[0,6]
#print (df.iloc[0,10])
#print (df)
df.index = np.arange(0, len(df))
df.to_csv (output+"-mean.csv", index = True, header=False)
#f = open("data.tmp.csv","w")
#f.write(dataset)
#f.close()


#dataset.index = np.arange(0, len(dataset))
#dataset.to_csv (output+"-mean.csv", index = True, header=False)
