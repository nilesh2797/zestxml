import sys
import os
import pdb
import operator

ft_index = {}
with open("ftIndex.txt","r") as f:
	line = f.readline()
	for line in f:
		items = line.rstrip().split("\t")
		ft_index[int(items[1])] = items[0]

lbl_index = {}
with open("lbIndex.txt","r") as f:
	line = f.readline()
	for line in f:
		items = line.rstrip().split("\t")
		lbl_index[int(items[1])] = items[0]

ft_file = open("trnFtMat.0.csv","r")
lbl_file = open("trnLblMat.0.csv","r")

lbl_dic = {}
ctr = 0
line = lbl_file.readline()
for line in lbl_file:
	if ctr%1000==0:
		print ctr
	ctr += 1
	items = line.strip().split(" ")
	for item in items:
		idval = item.split(":")
		if lbl_dic.has_key(int(idval[0])):
			lbl_dic[int(idval[0])] += 1
		else:
			lbl_dic[int(idval[0])] = 1

lbl_freq = open("lbl_freq.txt","w")
sorted_lbl_dic = sorted(lbl_dic.items(), key=operator.itemgetter(1))
sorted_lbl_dic.reverse()
#pdb.set_trace()
for item in sorted_lbl_dic:
	lbl_freq.write("%s\t%s\t%d\n" %(item[0],lbl_index[item[0]],item[1]))
lbl_freq.close()



ft_dic = {}
ctr = 0
line = ft_file.readline()
for line in ft_file:
	if ctr%1000==0:
		print ctr
	ctr += 1
	if line.strip()=="":
		continue
	items = line.strip().split(" ")
	for item in items:
		idval = item.split(":")
		if ft_dic.has_key(int(idval[0])):
			ft_dic[int(idval[0])] += 1
		else:
			ft_dic[int(idval[0])] = 1


ft_freq = open("ft_freq.txt","w")
sorted_ft_dic = sorted(ft_dic.items(), key=operator.itemgetter(1))
sorted_ft_dic.reverse()
#pdb.set_trace()
for item in sorted_ft_dic:
	ft_freq.write("%s\t%s\t%d\n" %(item[0],ft_index[item[0]],item[1]))
ft_freq.close()



ft_file.close()
lbl_file.close()
