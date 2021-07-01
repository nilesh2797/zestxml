import sys
import os
import pdb

ft_file = open("trnFtMat.0.csv","r")
lbl_file = open("trnLblMat.0.csv","r")
data_file = open("data.txt","w")

items = ft_file.readline().strip().split()
num_inst = int(items[0])
num_ft = int(items[1])
items = lbl_file.readline().strip().split()
num_lbl = int(items[1])

print >>data_file, "%d %d %d" %(num_inst,num_ft,num_lbl)

for i in range(num_inst):
	if i%1000==0:
		print i,num_inst
	ft_line = ft_file.readline()
	lbl_line = lbl_file.readline()
	items = lbl_line.strip().split()
	lbls = []
	for item in items:
		idval = item.split(":")
		lbls.append(idval[0])
	data_file.write("%s %s" %(",".join(lbls),ft_line))
		
ft_file.close()
lbl_file.close()
data_file.close()
