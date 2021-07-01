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

title_index = []
with open("titles.csv","r") as f:
	for line in f:
		title = line.strip()
		title_index.append(title)

pdb.set_trace()

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


ft_file.close()
lbl_file.close()

ft_th = 3
lbl_th = 2

valid_ft = []
for key,val in ft_dic.items():
	if val>=ft_th:
		valid_ft.append((key,ft_index[key]))

valid_lbl = []
for key,val in lbl_dic.items():
	if val>=lbl_th:
		valid_lbl.append((key,lbl_index[key]))

valid_ft = sorted(valid_ft,key=lambda x: x[1])
valid_lbl = sorted(valid_lbl,key=lambda x:x[1])

features_file = open("features.txt","w")
labels_file = open("labels.txt","w")
titles_file = open("titles.txt","w")
data_file = open("data.txt","w")

ft_old_to_new = {}
lbl_old_to_new = {}

ctr = 0
for item in valid_ft:
	ft_old_to_new[item[0]] = ctr
	print >>features_file, item[1]
	ctr += 1

ctr = 0
for item in valid_lbl:
	lbl_old_to_new[item[0]] = ctr
	print >>labels_file, item[1]
	ctr += 1


ft_file = open("trnFtMat.0.csv","r")
lbl_file = open("trnLblMat.0.csv","r")

items = ft_file.readline().strip().split()
num_inst = int(items[0])
num_ft = int(items[1])
items = lbl_file.readline().strip().split()
num_lbl = int(items[1])

for i in range(num_inst):
	if i%1000==0:
		print "data %d" %(i)

	ft_line = ft_file.readline()
	lbl_line = lbl_file.readline()

	lbls = []
	items = lbl_line.strip().split()
	for item in items:
		idval = item.split(":")
		if lbl_old_to_new.has_key(int(idval[0])):
			lbls.append(lbl_old_to_new[int(idval[0])])
	
	if not lbls:
		continue

	lbls = sorted(lbls)
	
	fts = []
	items = ft_line.strip().split()
	for item in items:
		idval = item.split(":")
		if ft_old_to_new.has_key(int(idval[0])):
			fts.append((ft_old_to_new[int(idval[0])],idval[1]))

	if not fts:
		continue

	fts = sorted(fts,key=lambda x: x[0])

	print >>titles_file, title_index[i]
		
	print >>data_file, ",".join(map(lambda x:str(x),lbls))+" "+" ".join(map(lambda x: "%d:%s" %(x[0],x[1]) ,fts) )

features_file.close()
labels_file.close()
titles_file.close()
data_file.close()
pdb.set_trace()
