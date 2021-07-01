import sys
import os
from pprint import pprint

inst_ft_map = []
inst_lbl_map = []

data_folder = sys.argv[1]
data_file = open(data_folder+"/data.txt","r")

ctr = 0
for line in data_file:
    inst_lbl_map.append({})
    inst_ft_map.append({})

    items = line.rstrip().split(' ')
    if len(items[0])>0:
        lbls = items[0].strip().split(',')
        for lbl in lbls:
            inst_lbl_map[ctr][int(lbl)] = 1
    for item in items[1:]:
        ftval = item.strip().split(':')
        inst_ft_map[ctr][int(ftval[0])] = 1

    #pprint(inst_lbl_map[ctr])
    #pprint(inst_ft_map[ctr])

    ctr = ctr+1

    #ch = sys.stdin.readline()
data_file.close()

num_trn = 0
num_tst = 0
trn_file = open(data_folder+"/trn.0.txt","r")
for line in trn_file:
    num_trn = num_trn+1
trn_file.close()
tst_file = open(data_folder+"/tst.0.txt","r")
for line in tst_file:
    num_tst = num_tst+1
tst_file.close()

num_inst = len(inst_ft_map)
num_ft = -1
num_lbl = -1
ft_per_inst = 0
lbl_per_inst = 0
inst_per_ft = 0
inst_per_lbl = 0

for ft_map in inst_ft_map:
    count = 0
    for key,val in ft_map.items():
        count = count+1
        if key>num_ft:
            num_ft = key
    ft_per_inst = ft_per_inst + count
num_ft = num_ft+1

for lbl_map in inst_lbl_map:
    count = 0
    for key,val in lbl_map.items():
        count = count+1
        if key>num_lbl:
            num_lbl = key
    lbl_per_inst = lbl_per_inst + count
num_lbl = num_lbl+1

inst_per_ft = 1.0*ft_per_inst/num_ft
inst_per_lbl = 1.0*lbl_per_inst/num_lbl
ft_per_inst = 1.0*ft_per_inst/num_inst
lbl_per_inst = 1.0*lbl_per_inst/num_inst

meta_file = open(data_folder+"/metadata.txt","w")
print >>meta_file, "Num-Train\t%d" %(num_trn)
print >>meta_file, "Num-Test\t%d" %(num_tst)
print >>meta_file, "Num-Features\t%d" %(num_ft)
print >>meta_file, "Num-Labels\t%d" %(num_lbl)
print >>meta_file, "Avg. Feat-per-Inst\t%f" %(ft_per_inst)
print >>meta_file, "Avg. Inst-per-Feat\t%f" %(inst_per_ft)
print >>meta_file, "Avg. Lbl-per-Inst\t%f" %(lbl_per_inst)
print >>meta_file, "Avg. Inst-per-Lbl\t%f" %(inst_per_lbl)
meta_file.close()
