import sys
import os
import pdb
import random

data_folder_name = sys.argv[1]
num_inst = int(sys.argv[2])
num_splits = int(sys.argv[3])
frac = float(sys.argv[4])

num_trn = int(frac*num_inst)
num_tst = num_inst - num_trn

#print num_trn,num_tst

for i in range(num_splits):
	trn_file = open(data_folder_name+"/trn.%d.txt" %(i),"w")
	tst_file = open(data_folder_name+"/tst.%d.txt" %(i),"w")

	inds = range(num_inst)
	random.shuffle(inds)
	trn_inds = inds[:num_trn]
	tst_inds = inds[num_trn:]

	trn_inds.sort()
	tst_inds.sort()

	for ind in trn_inds:
		print >>trn_file, ind

	for ind in tst_inds:
		print >>tst_file, ind

	trn_file.close()
	tst_file.close()

