# creates a single train/test split such that each label is present in both train and test data as much as possible

import os
import sys
import pdb
import random

train_lbls = {}
test_lbls = {}
train_insts = {}
test_insts = {}

data_str = "data.txt"
data_file = open(data_str,"r")
ctr = 0
for line in data_file:
    if ctr%100==0:
        print ctr

    items = line.rstrip().split(" ",1)
    lbls = items[0].strip().split(",")
    for lbl in lbls:
        lbl = int(lbl)
        if not(train_lbls.has_key(lbl)):
            for lbl1 in lbls:
                lbl1 = int(lbl1)
                train_lbls[lbl1] = True
            train_insts[ctr] = True
            break
        elif not(test_lbls.has_key(lbl)):
            for lbl1 in lbls:
                lbl1 = int(lbl1)
                test_lbls[lbl1] = True
            test_insts[ctr] = True
            break
    ctr += 1

data_file.close()

print len(train_insts),len(test_insts)
print len(train_lbls),len(test_lbls)

frac = 0.8

data_file = open(data_str,"r")
ctr = 0
for line in data_file:
    if ctr%100==0:
        print ctr

    if not(train_insts.has_key(ctr)) and not(test_insts.has_key(ctr)):
        if random.random()<0.8:
            train_insts[ctr] = True
        else:
            test_insts[ctr] = True
    
    ctr += 1

data_file.close()

trn_inds = train_insts.keys()
trn_inds.sort()
tst_inds = test_insts.keys()
tst_inds.sort()

trn_file = open("trn.0.txt","w")
tst_file = open("tst.0.txt","w")

for ind in trn_inds:
    print >>trn_file, ind

for ind in tst_inds:
    print >>tst_file, ind

trn_file.close()
tst_file.close()
