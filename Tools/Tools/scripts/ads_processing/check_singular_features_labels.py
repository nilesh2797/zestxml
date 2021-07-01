import sys
import os
import pdb

labels = {}
features = {}

#in_file = open("/media/yashoteja/Storage/Learning/Research/Algorithms/Data/Datasets/Ads-Small/data.txt","r")
in_file = open("/media/yashoteja/Storage/Learning/Research/Algorithms/Data/Raw/Ads_Abhirup/Ads-Large/original_data.txt","r")

ctr = 0
for line in in_file:
    if ctr%100==0:
        print ctr

    items = line.rstrip().split()
    if items[0]!="":
        lbls = items[0].split(",")
        for lbl in lbls:
            if int(lbl) in labels:
                labels[int(lbl)] += 1
            else:
                labels[int(lbl)] = 1

    for item in items[1:]:
        idval = item.split(":")
        ftind = int(idval[0])
        if ftind in features:
            features[ftind] += 1
        else:
            features[ftind] = 1

    ctr += 1

in_file.close()

tot = 0
sing = 0
for k,v in features.items():
    tot += 1
    if v==1:
        sing += 1

print tot,sing

tot = 0
sing = 0
for k,v in labels.items():
    tot += 1
    if v==1:
        sing += 1

print tot,sing
