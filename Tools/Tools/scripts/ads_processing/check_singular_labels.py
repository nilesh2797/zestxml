import sys
import os
import pdb

lbl_counts = {}
ctr = 0
data_file = open("data.txt","r")
for line in data_file:
	ctr += 1
	if ctr%100==0:
		print ctr

	lbls = line.split(" ",1)
	lbls = lbls[0]
	lbls = lbls.split(",")
	for lbl in lbls:
		if lbl in lbl_counts:
			lbl_counts[lbl] += 1
		else:
			lbl_counts[lbl] = 1
data_file.close()

singular_count = 0
for lbl,count in lbl_counts.items():
	if count==1:
		singular_count += 1

num_valid = 0
ctr = 0
data_file = open("data.txt","r")
for line in data_file:
	ctr += 1
	if ctr%100==0:
		print ctr

	valid = False
	lbls = line.split(" ",1)
	lbls = lbls[0]
	lbls = lbls.split(",")
	for lbl in lbls:
		if lbl_counts[lbl]>1:
			valid = True
	if valid:
		num_valid += 1
data_file.close()

print len(lbl_counts),singular_count
print ctr,num_valid
