import sys
import os
import pdb

ft_th = 1
lbl_th = 1

data_file_str = "/media/yashoteja/Storage/Learning/Research/Algorithms/Data/Raw/Wiki_Stemmed_without_stubs/outfile_WithStem.txt"
titles = []
count_fts = {}
count_lbls = {}
data_file = open(data_file_str,"r")
ctr = 0;
for line in data_file:
	ctr += 1
	if ctr%100==0:
		print ctr

	"""
	if ctr==1000:
		break
	"""

	items = line.strip().split("\t")
	titles.append(items[0])

	if items[1]=="":
		continue
	lbls = items[1].split(",")
	for lbl in lbls:
		lbl = lbl.strip()
		if count_lbls.has_key(lbl):
			count_lbls[lbl] += 1
		else:
			count_lbls[lbl] = 1
	
	if items[2]=="":
		continue
	fts = items[2].split(",")
	for ft in fts:
		idval = ft.split(":")
		idval[0] = idval[0].strip()
		if  idval[0] in count_fts:
			count_fts[idval[0]] += 1
		else:
			count_fts[idval[0]] = 1

data_file.close()

valid_fts = []
for ft,count in count_fts.items():
	if count>ft_th:
		valid_fts.append(ft)
print len(count_fts),len(valid_fts)
valid_fts.sort()
ft_map = dict(zip(valid_fts,range(len(valid_fts))))

valid_lbls = []
for lbl,count in count_lbls.items():
	if count>lbl_th:
		valid_lbls.append(lbl)
print len(count_lbls),len(valid_lbls)
valid_lbls.sort()
lbl_map = dict(zip(valid_lbls,range(len(valid_lbls))))

#pdb.set_trace()

data_file = open(data_file_str,"r")
new_data_file = open("data.txt","w")
inst_index_file = open("instances.txt","w")
lbl_index_file = open("labels.txt","w")
ft_index_file = open("features.txt","w")

for i in range(len(valid_lbls)):
	print >>lbl_index_file, "%d\t%s" %(i,valid_lbls[i])

for i in range(len(valid_fts)):
	print >>ft_index_file, "%d\t%s" %(i,valid_fts[i])

ctr = 0
valid_ctr = 0
for line in data_file:
	ctr += 1 
	if ctr%100==0:
		print ctr

	"""
	if ctr==1000:
		break
	"""

	valid = False

	items = line.strip().split("\t")
	if items[1]=="":
		continue
	lbls = items[1].split(",")
	valid_lbls_vec = []
	for lbl in lbls:
		lbl = lbl.strip()
		if count_lbls[lbl]>lbl_th:
			valid = True
			valid_lbls_vec.append(lbl_map[lbl])

	if valid:
		valid_fts_vec = []
		valid = False
		fts = items[2].split(",")
		for ft in fts:
			idval = ft.split(":")
			idval[0] = idval[0].strip()
			ftind = idval[0]
			if count_fts[ftind]>ft_th:
				valid = True
				valid_fts_vec.append((ft_map[ftind],int(idval[1])))

	if valid:
		valid_lbls_vec.sort()
		valid_fts_vec.sort()

		print >>inst_index_file, "%d\t%s" %(valid_ctr,titles[ctr-1])
		valid_ctr += 1
		new_data_file.write("%s %s\n" %(",".join(map(lambda x: str(x),valid_lbls_vec)), " ".join(map(lambda x:"%d:%d" %(x[0],x[1]),valid_fts_vec))))	


data_file.close()
new_data_file.close()
inst_index_file.close()
lbl_index_file.close()
ft_index_file.close()
